import sqlite3
import csv
import json
from datetime import datetime
from sqlite3 import Error

db_location = "db/r2_course.db"
sql_create_droids_table = """ CREATE TABLE IF NOT EXISTS droids (
                                        droid_uid integer PRIMARY KEY,
                                        name text NOT NULL,
                                        member text NOT NULL,
                                        material text NULL,
                                        weight text NULL,
                                        transmitter_type text NULL,
                                        new boolean NO
                                    ); """

sql_create_members_table = """ CREATE TABLE IF NOT EXISTS members (
                                        member_uid integer PRIMARY KEY,
                                        name text NOT NULL,
                                        email text NULL,
                                        new boolean NO
                                    ); """

sql_create_gates_table = """ CREATE TABLE IF NOT EXISTS gates (
                                        id integer PRIMARY KEY,
                                        type text NOT NULL,
                                        name text NOT NULL,
                                        penalty integer NOT NULL
                                    ); """

sql_create_runs_table = """ CREATE TABLE IF NOT EXISTS runs (
                                        id integer PRIMARY KEY,
                                        start DATETIME DEFAULT CURRENT_TIMESTAMP,
                                        middle_start DATETIME NULL,
                                        droid_uid integer NOT NULL,
                                        member_uid integer NOT NULL,
                                        first_half_time integer NULL,
                                        second_half_time integer NULL,
                                        clock_time integer NULL,
                                        final_time integer NULL,
                                        type text NULL
                                    ); """

sql_create_penalties_table = """ CREATE TABLE IF NOT EXISTS penalties (
                                        id integer PRIMARY KEY,
                                        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                                        run_id integer NOT NULL,
                                        gate_id integer NOT NULL
                                    ); """

sql_create_course_table = """ CREATE TABLE IF NOT EXISTS course (
                                        config_name text PRIMARY KEY,
                                        config_value text NULL 
                                    ); """


def db_init():
    conn = create_connection(db_location)
    if conn is not None:
        execute_sql(conn, sql_create_droids_table)
        execute_sql(conn, sql_create_members_table)
        execute_sql(conn, sql_create_runs_table)
        execute_sql(conn, sql_create_penalties_table)
        execute_sql(conn, sql_create_course_table)
        if get_config('course_type') is None:
            # Best load initial config values:
            if __debug__:
                print("Loading initial config values to course table")
            with open ('db/config.csv', 'rt') as fin:
                dr = csv.DictReader(fin)
                to_db = [(i['config_name'], i['config_value']) for i in dr]
            c = conn.cursor()
            c.executemany("INSERT INTO course (config_name, config_value) VALUES (?, ?);", to_db)
            conn.commit()
        load_gates()
    else:
        print("Error!")

def create_connection(db_file):
    """ create a database connection to a SQLite database """
    try:
        conn = sqlite3.connect(db_file)
        return conn
    except Error as e:
        print(e)
    return None

def get_config(setting):
    conn = create_connection(db_location)
    c = conn.cursor()
    c.execute("SELECT config_value FROM course WHERE config_name = \"" + str(setting) + "\";")
    row = c.fetchone()
    if row is None:
        return None
    else:
        return row[0] 

def set_config(setting, value):
    conn = create_connection(db_location)
    execute_sql(conn, "UPDATE course SET config_value=\"" + value + "\" WHERE config_name=\"" + setting + "\";")
    return 


def load_gates():
    """ Load gate config """
    gates_csv = "course/" + get_config('course_type') + "/sensors.csv"
    with open (gates_csv, 'rt') as fin:
        dr = csv.DictReader(fin)
        to_db = [(i['id'], i['type'], i['name'], i['penalty']) for i in dr]

    conn = create_connection(db_location)
    execute_sql(conn, "DROP TABLE gates")
    execute_sql(conn, sql_create_gates_table)
    c = conn.cursor()
    c.executemany("INSERT INTO gates (id, type, name, penalty) VALUES (?, ?, ?, ?);", to_db)
    conn.commit()
    return None

def execute_sql(conn, sql):
    """ run an sql statement
    :param conn: Connection object
    :param sql: an SQL statement
    :return:
    """
    try:
        c = conn.cursor()
        if __debug__:
            print("Executing SQL: %s" % sql)
        c.execute(sql)
    except Error as e:
        print(e)
    conn.commit()

def clear_db(table):
    conn = create_connection(db_location)
    if table != "all":
        execute_sql(conn, "DELETE FROM " + table)
    else: 
        execute_sql(conn, "DELETE FROM runs")
        execute_sql(conn, "DELETE FROM penalties")

def run(current, cmd, member_id, droid_id, milliseconds):
    conn = create_connection(db_location)
    c = conn.cursor()
    if cmd == 'START':
        c.execute("INSERT INTO runs (droid_uid, member_uid) VALUES (" + str(droid_id) + ", " + str(member_id) + ");")
        conn.commit()
        run_id = c.lastrowid
        c.execute("DELETE FROM penalties WHERE run_id = " + str(run_id) + ";")
        conn.commit()
        if __debug__:
            print("Run ID: %s" % run_id)
        return run_id
    if cmd == 'MIDDLE_WAIT':
        c.execute("UPDATE runs SET first_half_time = " + milliseconds + " WHERE id = " + str(current) + ";")
    if cmd == 'MIDDLE_START':
        c.execute("UPDATE runs SET middle_start = current_timestamp WHERE id = " + str(current) + ";")
    if cmd == 'FINISH':
        penalty_time = 0
        # Work out clock time
        c.execute("SELECT first_half_time FROM runs WHERE id = " + str(current) + ";")
        row = c.fetchone()
        first_half = int(row[0])
        second_half = int(milliseconds) - int(row[0])
        clock_time = int(milliseconds)
        if __debug__:
            print("First half: %s | Second half: %s | Total: %s" % (first_half, second_half, clock_time))
        # Work out penalties
        c.execute("SELECT gate_id FROM penalties WHERE run_id = " + str(current) + ";")
        penalties = c.fetchall()
        for penalty in penalties:
            c.execute("SELECT penalty FROM gates WHERE id = " + str(penalty[0]) + ";")
            row = c.fetchone()
            penalty_time += row[0]
            if __debug__:
                print("Penalty found on gate: %s | Penalty added: %s | Total Penalty time: %s" % (penalty[0], row[0], penalty_time))
        # Add penalties to clock_time and update database
        final_time = clock_time + (penalty_time * 1000)
        c.execute("UPDATE runs SET second_half_time = " + str(second_half) +
                ", clock_time = " + str(clock_time) + ", final_time = " + str(final_time) + " WHERE id = " + str(current) + ";")

    conn.commit()
    return 

def current_run(run_id):
    """ Get current run details from database """
    conn = create_connection(db_location)
    c = conn.cursor()
    c.execute("SELECT * FROM runs WHERE id = " + str(run_id))
    result = c.fetchone()
    if result is not None:
        run = dict((c.description[i][0], value) for i, value in enumerate(result))
        run['penalties'], run['num_penalties'] = list_penalties(run_id);
    else:
        run = {'id': 0, 'start': None, 'middle_stop': None, 'middle_start': None, 'end': None, 'droid_uid': 0, 'member_uid': 0, 'first_half_time': None, 'second_half_time': None, 'clock_time': None, 'final_time': None, 'num_penalties': 0}
    if __debug__:
        print(run)
    conn.commit()
    conn.close()
    return run

def is_top(run_id):
    """ Checks if run_id is the top score """
    conn = create_connection(db_location)
    c = conn.cursor()
    c.execute("SELECT id from runs WHERE final_time != \"\" order by final_time ASC LIMIT 1")
    result = c.fetchone()
    if __debug__:
        print(result)
    if result[0] != run_id:
        return "no"
    else:
        return "yes" 
       

def log_penalty(gate_id, run_id):
    conn = create_connection(db_location)
    if conn is not None:
        sql = "INSERT INTO penalties (gate_id, run_id) VALUES (" + str(gate_id) + ", " + str(run_id) + ");"
        if __debug__:
            print("SQL: %s" % sql)
        execute_sql(conn, sql)
    else:
        print("Could not log penalty")
        
    return

def get_member(did):
    """ Get member details from database """
    conn = create_connection(db_location)
    c = conn.cursor()
    c.execute("SELECT * FROM members WHERE member_uid = " + did)
    member = dict((c.description[i][0], value) for i, value in enumerate(c.fetchone()))
    if __debug__:
        print(member)
    conn.commit()
    conn.close()
    return member

def add_member(data):
    """ Add a member to the database """
    print("Adding: %s " % data)
    conn = create_connection(db_location)
    sql = "INSERT INTO members(member_uid, name, email, new) VALUES({}, \"{}\", \"{}\", \"{}\");".format(
            data['member_uid'], data['forename'] + " " + data['surname'], data['email'], data['new'])
    execute_sql(conn, sql)
    return

def get_droid(did):
    """ Get droid details from database """
    conn = create_connection(db_location)
    c = conn.cursor()
    c.execute("SELECT * FROM droids WHERE droid_uid = " + did)
    droid = dict((c.description[i][0], value) for i, value in enumerate(c.fetchone()))
    if __debug__:
        print(droid)
    conn.commit()
    conn.close()
    return (droid)

def add_droid(data):
    """ Add a droid to the database """
    print("Adding: %s " % data)  
    conn = create_connection(db_location)
    sql = "INSERT INTO droids(droid_uid, name, member, material, weight, transmitter_type, new) VALUES({}, \"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\");".format(
            data['droid_uid'], data['name'], data['member_uid'], data['material'], data['weight'], data['transmitter_type'], data['new'])
    execute_sql(conn, sql)
    return

def list_gates():
    conn = create_connection(db_location)
    conn.row_factory = sqlite3.Row
    c = conn.cursor()
    c.execute("SELECT * FROM gates;")
    gates = [dict(row) for row in c.fetchall()]
    if __debug__:
        print("Gates: %s" % gates)
    return gates

def list_penalties(runid):
    conn = create_connection(db_location)
    c = conn.cursor()
    num_penalties = 0
    penalties = {}
    gates = list_gates()
    for gate in gates:
        if __debug__:
            print("Checking gate: %s" % gate['name'])
        c.execute("SELECT * FROM penalties WHERE gate_id = " + str(gate['id']) + " AND run_id = " + str(runid))
        penalty = c.fetchone()
        if penalty is not None:
            if __debug__:
                print("Fail on gate %s " % gate['name'])
            penalties[gate['id']] = gate['penalty']
            num_penalties += 1
        else:
            if __debug__:
                print("Pass on gate %s " % gate['name'])
            penalties[gate['id']] = "0"
    if __debug__:
        print("Penalties: %s" % penalties)
        print("Number of Penalties: %s" % num_penalties)
    return penalties, num_penalties


def list_results():
    results = []
    conn = create_connection(db_location)
    c = conn.cursor()
    c.execute("SELECT id, start, droid_uid, member_uid, first_half_time, second_half_time, clock_time, final_time FROM runs WHERE final_time != \"\" ORDER BY final_time ASC;")
    runs = c.fetchall()
    for run in runs:
        data = {}
        if __debug__:
            print("Run: %s " % run[0])
        member = get_member(str(run[3]))
        droid = get_droid(str(run[2]))
        data['member'] = member['name']
        data['droid'] = droid['name']
        data['start'] = run[1]
        data['first_half'] = run[4]
        data['second_half'] = run[5]
        data['clock_time'] = run[6]
        data['final_time'] = run[7]
        data['penalties'], data['num_penalties'] = list_penalties(run[0])
        if __debug__:
            print(data)
        results.append(data)
    return json.dumps(results)

def list_today():
    results = []
    conn = create_connection(db_location)
    c = conn.cursor()
    c.execute("SELECT id, start, droid_uid, member_uid, first_half_time, second_half_time, clock_time, final_time FROM runs WHERE final_time != \"\" AND start > date('now', 'start of day') ORDER BY final_time ASC;")
    runs = c.fetchall()
    for run in runs:
        data = {}
        if __debug__:
            print("Run: %s " % run[0])
        member = get_member(str(run[3]))
        droid = get_droid(str(run[2]))
        data['member'] = member['name']
        data['droid'] = droid['name']
        data['start'] = run[1]
        data['first_half'] = run[4]
        data['second_half'] = run[5]
        data['clock_time'] = run[6]
        data['final_time'] = run[7]
        data['penalties'], data['num_penalties'] = list_penalties(run[0])
        if __debug__:
            print(data)
        results.append(data)
    return json.dumps(results)

def list_runs():
    results = []
    conn = create_connection(db_location)
    c = conn.cursor()
    c.execute("SELECT id, start, droid_uid, member_uid, first_half_time, second_half_time, clock_time, final_time FROM runs WHERE final_time != \"\" ORDER BY final_time ASC;")
    runs = c.fetchall()
    for run in runs:
        data = {}
        if __debug__:
            print("Run: %s " % run[0])
        data['id'] = run[0]
        data['member_uid'] = run[3]
        data['droid_uid'] = run[2]
        data['start'] = run[1]
        data['first_half'] = run[4]
        data['second_half'] = run[5]
        data['clock_time'] = run[6]
        data['final_time'] = run[7]
        data['penalties'], data['num_penalties'] = list_penalties(run[0])
        if __debug__:
            print(data)
        results.append(data)
    return json.dumps(results)

def list_members():
    results = []
    conn = create_connection(db_location)
    c = conn.cursor()
    c.execute("SELECT * FROM members")
    members = c.fetchall()
    for member in members:
        data = {}
        if __debug__:
            print("Member: %s " % member[0])
        data['member_uid'] = member[0]
        data['name'] = member[1]
        if __debug__:
            print(data)
        results.append(data)
    return json.dumps(results)

def list_droids():
    results = []
    conn = create_connection(db_location)
    c = conn.cursor()
    c.execute("SELECT * FROM droids, members WHERE droids.member = members.member_uid")
    droids = c.fetchall()
    for droid in droids:
        data = {}
        if __debug__:
            print("droid: %s " % droid[0])
        data['droid_uid'] = droid[0]
        data['droid_name'] = droid[1]
        data['member_name'] = droid[8]
        if __debug__:
            print(data)
        results.append(data)
    return json.dumps(results)


def delete_run(run_id):
    conn = create_connection(db_location)
    c = conn.cursor()
    c.execute("DELETE FROM runs WHERE id = " + str(run_id));
    conn.commit()
    conn.close()
    return "Ok"


if __name__ == '__main__':
    create_connection(db_location)

