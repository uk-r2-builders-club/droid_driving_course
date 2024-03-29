#!/usr/bin/python3
""" R2 Builder Obstacle Course """
from __future__ import print_function
import os
import time
import datetime
import urllib.request
import json
import ast
import serial
import requests
from pathlib import Path
from collections import namedtuple
from future import standard_library
from flask import Flask, request, render_template
from flask_socketio import SocketIO, emit
import database 
import broadcast
import audio


database.db_init()

Droid = namedtuple('Droid', 'droid_uid, member_uid, name, material, weight, transmitter_type')
Driver = namedtuple('Driver', 'member_uid, name, email')

api_key = database.get_config('mot_api_key')
site_base = database.get_config('mot_site_base')

current_droid = Droid(droid_uid=0, member_uid=0, name="none", material="none", weight="none", transmitter_type="none")
current_member = Driver(member_uid=0, name="none", email="none")
current_run = 0
current_state = 0

app = Flask(__name__, template_folder='templates')
app.config['key'] = database.get_config('app_key')
socketio = SocketIO(app)
try:
    ser = serial.Serial('/dev/ttyUSB0')
except:
    print("No serial device")

broadcast = broadcast.BroadCaster()

audio = audio.AudioLibrary("sounds", 1)

def course_types():
    subdirs = os.listdir("course")
    return json.dumps(subdirs)

@app.route('/')
def index():
    """GET to generate a list of endpoints and their docstrings"""
    urls = dict([(r.rule, app.view_functions.get(r.endpoint).__doc__)
                 for r in app.url_map.iter_rules()
                 if not r.rule.startswith('/static')])
    return render_template('index.html', urls=urls)

@app.route('/scoreboard')
def scoreboard():
    return render_template('scoreboard.html', async_mode=socketio.async_mode)

@app.route('/results')
def results():
    return render_template('results.html', async_mode=socketio.async_mode)

@app.route('/today')
def today():
    return render_template('today.html', async_mode=socketio.async_mode)

@app.route('/contenders')
def contenders():
    return render_template('contenders.html', async_mode=socketio.async_mode)


@app.route('/display/<cmd>')
def display(cmd):
    global current_droid, current_member
    if request.method == 'GET':
        if cmd == 'results':
            return database.list_results()
        if cmd == 'today':
            return database.list_today()
        if cmd == 'contender':
            contender = {}
            contender['member_uid'] = current_member.member_uid
            contender['member'] = current_member.name
            contender['droid_uid'] = current_droid.droid_uid
            contender['droid'] = current_droid.name
            contender['material'] = current_droid.material
            contender['weight'] = current_droid.weight
            contender['transmitter_type'] = current_droid.transmitter_type
            return json.dumps(contender)
        if cmd == 'current':
            return json.dumps(database.current_run(current_run))
        if cmd == 'list_gates':
            gates = json.dumps(database.list_gates())
            if __debug__:
                print("Gates: %s" % gates)
            return gates
        if cmd == 'current_gates':
            return json.dumps(database.list_penalties(current_run))
        if cmd == 'droids':
            return database.list_droids()
        if cmd == 'members':
            return database.list_members()
        if cmd == 'course_types':
            return course_types()
    return "Ok"

@app.route('/droid/<did>', methods=['GET'])
def droid_register(did):
    global current_droid, current_run, current_state
    if request.method == 'GET':
        broadcast.broadcast_message(b'reset')
        data = json.dumps(database.get_droid(did))
        current_droid = json.loads(data, object_hook=lambda d: namedtuple('Droid', d.keys())(*d.values()))
        print(current_droid)
        print("Droid Registered: %s" % did)
        current_run = 0
        current_state = 0
        socketio.emit('my_response', {'data': 'Droid Registered'}, namespace='/comms')
        socketio.emit('reload_contender', {'data': 'reload contender'}, namespace='/comms')
        socketio.emit('reload_results', {'data': 'reload results'}, namespace='/comms')
        socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
        socketio.emit('reload_gates', {'data': 'reload current'}, namespace='/comms')
    return "Ok"

@app.route('/member/<did>', methods=['GET'])
def member_register(did):
    global current_member, current_run, current_state
    if request.method == 'GET':
        broadcast.broadcast_message(b'reset')
        data = json.dumps(database.get_member(did))
        current_member = json.loads(data, object_hook=lambda d: namedtuple('Driver', d.keys())(*d.values()))
        print(current_member)
        print("Driver Registered: %s" % did)
        current_run = 0
        current_state = 0 
        socketio.emit('my_response', {'data': 'Driver Registered'}, namespace='/comms')
        socketio.emit('reload_contender', {'data': 'reload contender'}, namespace='/comms')
        socketio.emit('reload_results', {'data': 'reload results'}, namespace='/comms')
        socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
        socketio.emit('reload_gates', {'data': 'reload current'}, namespace='/comms')
    return "Ok"

@app.route('/gate/<gid>/<value>', methods=['GET'])
def gate_trigger(gid, value):
    global current_run, current_state
    if request.method == 'GET':
        if value == 'FAIL':
            if current_run != 0 and current_state != 4:
               audio.TriggerSound("woop_woop")
               database.log_penalty(gid, current_run)
               socketio.emit('my_response', {'data': 'PENALTY!!!'}, namespace='/comms')
               socketio.emit('reload_gates', {'data': 'reload current'}, namespace='/comms')
               socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
        else:
            print("Gate passed")
        print("Gate ID: %s | %s" % (gid, value))
    return "Ok"

@app.route('/run/<cmd>/<milliseconds>', methods=['GET'])
def run_cmd(cmd, milliseconds):
    global current_run, current_member, current_droid, current_state
    if request.method == 'GET':
        if __debug__:
            print("Current run: %s | Current Driver: %s | Current Droid: %s " % (current_run, current_member.member_uid, current_droid.droid_uid))
        if cmd == 'START' and current_member.member_uid != 0 and current_state == 0:
            current_run = database.run(0, cmd, current_member.member_uid, current_droid.droid_uid, 0)
            socketio.emit('my_response', {'data': 'Start Run'}, namespace='/comms')
            socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
            audio.TriggerSound("air_horn")
            current_state = 1
        if cmd == 'MIDDLE_WAIT' and current_state == 1:
            database.run(current_run, cmd, current_member.member_uid, current_droid.droid_uid, milliseconds)
            socketio.emit('my_response', {'data': 'Halfway Rest'}, namespace='/comms')
            socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
            current_state = 2
        if cmd == 'MIDDLE_START' and current_state == 2:
            database.run(current_run, cmd, current_member.member_uid, current_droid.droid_uid, 0)
            socketio.emit('my_response', {'data': 'Continuing Run'}, namespace='/comms')
            socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
            current_state = 3
        if cmd == 'FINISH' and current_state == 3:
            database.run(current_run, cmd, current_member.member_uid, current_droid.droid_uid, milliseconds)
            socketio.emit('my_response', {'data': 'Finish!'}, namespace='/comms')
            socketio.emit('reload_results', {'data': 'reload results'}, namespace='/comms') 
            socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
            run_details = database.current_run(current_run)
            if __debug__:
                print("**** Run Details: %s" % run_details)
            if database.is_top(current_run) == "yes":
                broadcast.broadcast_message(b'rainbow')
                socketio.emit('special_display', {'data': 'toprun'}, namespace='/comms')
                socketio.emit('my_response', {'data': '**** TOP RUN ****'}, namespace='/comms')
            if run_details["final_time"] > 120000:
                socketio.emit('special_display', {'data': 'slow'}, namespace='/comms')
                socketio.emit('my_response', {'data': '**** SLOOOOOOOOOOW ****'}, namespace='/comms')
            if run_details["num_penalties"] > 6:
                socketio.emit('special_display', {'data': 'pinball'}, namespace='/comms')
                socketio.emit('my_response', {'data': '**** PINBALL DROID ****'}, namespace='/comms')
            current_state = 4
        if cmd == 'RESET':
            current_run = 0
            current_state = 0
            current_droid = Droid(droid_uid=0, member_uid=0, name="none", material="none", weight="none", transmitter_type="none")
            current_member = Driver(member_uid=0, name="none", email="none")
            socketio.emit('my_response', {'data': 'Resetting'}, namespace='/comms')
            socketio.emit('reload_results', {'data': 'reload results'}, namespace='/comms')
            socketio.emit('reload_contender', {'data': 'reload contender'}, namespace='/comms')
            socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
            socketio.emit('reload_gates', {'data': 'reload current'}, namespace='/comms')
            broadcast.broadcast_message(b'reset')
        socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
    return "Ok"

@app.route('/admin')
def admin():
    return render_template('admin.html', async_mode=socketio.async_mode)

@app.route('/admin/display/<cmd>', methods=['GET'])
def special_display(cmd):
    if request.method == 'GET':
        socketio.emit('my_response', {'data': '**ADMIN** Special Display'}, namespace='/comms')
        socketio.emit('special_display', {'data': cmd}, namespace='/comms')
    return "Ok"

@app.route('/admin/clear_db', methods=['GET'])
def clear_db():
    if request.method == 'GET':
        database.clear_db("all")
        socketio.emit('my_response', {'data': '**ADMIN** Database cleared'}, namespace='/comms')
        socketio.emit('reload_results', {'data': 'reload results'}, namespace='/comms')
    return "Ok"

@app.route('/admin/change_course/<course>', methods=['GET'])
def change_course(course):
    if request.method == 'GET':
        database.set_config("course_type", course)
        database.load_gates()
        socketio.emit('my_response', {'data': '**ADMIN** Changing Course Type'}, namespace='/comms')
        socketio.emit('reload_results', {'data': 'reload results'}, namespace='/comms')
        socketio.emit('reload_gates', {'data': 'reload current'}, namespace='/comms')
        socketio.emit('course_change', {'data': 'course change'}, namespace='/comms')
    return "Ok"

@app.route('/admin/refresh/all', methods=['GET'])
def refresh_all():
    if request.method == 'GET':
        database.clear_db("members")
        database.clear_db("droids")
        url = site_base + "/api/getmembers" + "?api_token=" + api_key
        print(url)
        data = json.loads(ast.literal_eval(urllib.request.urlopen(url).read().decode('utf-8').rstrip('\n')))
        for member in data:
            member['new'] = 'no'
            database.add_member(member)
            image_url = site_base + "/api/getmemberimage/" + str(member['id']) + "?api_token=" + api_key
            print(image_url)
            urllib.request.urlretrieve(image_url, "static/members/" + str(member['id']) + ".jpg")
            for droid in member['droids']:
                droid['new'] = 'no'
                droid['member_uid'] = member['id']
                database.add_droid(droid)
                image_url = site_base + "/api/getdroidimage/" + str(droid['id']) + "?api_token=" + api_key
                print(image_url)
                urllib.request.urlretrieve(image_url, "static/droids/" + str(droid['id']) + ".jpg")
    return "Ok"

@app.route('/admin/refresh/scoreboard', methods=['GET'])
def refresh_scoreboard():
    if request.method == 'GET':
        socketio.emit('my_response', {'data': '**ADMIN** Scoreboard refreshed'}, namespace='/comms')
        socketio.emit('reload_contender', {'data': 'reload contender'}, namespace='/comms')
        socketio.emit('reload_results', {'data': 'reload results'}, namespace='/comms')
        socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
        socketio.emit('reload_gates', {'data': 'reload current'}, namespace='/comms')
    return "Ok"

@app.route('/admin/upload/runs', methods=['GET'])
def upload_runs():
    if request.method == 'GET':
        results = json.loads(database.list_runs())
        print(results)
        for result in results:
            if __debug__:
                print("Uploading Run ID: %s" % result['id'])
            url = site_base + "/api/uploadrun" #?api_token=" + api_key
            headers = { "Authorization" : "Bearer " + api_key, "Accept" : "application/json" }
            #code = urllib.request.urlopen(url).read().decode('utf-8').rstrip('\n')
            r = requests.post(url, data={"json": json.dumps(result)}, headers=headers)
            print(r.status_code)
            if r.status_code != 200:
                print("Error in uploading run")
            else: 
                print("Deleting row from local database")
                database.delete_run(result['id'])
    return "Ok"

@app.route('/admin/connected', methods=['GET'])
def list_connected():
    lease_file = open('/var/lib/misc/dnsmasq.leases', 'r')
    space = ' '
    message = '<!DOCTYPE html><html><body><h1>DHCP Leases</h1>\n<pre><code>' + \
                  'Expiry' + space * 4 + \
                  'MAC Address' + space * 8 + \
                  'IP Address' + space * 7 + \
                  'Hostname\n'
    for line in lease_file.readlines():
        columns = line.split()
        columns[0] = time.strftime('%H:%M:%S',
                                       time.localtime(int(columns[0])))
        lease = ''
        # discard last column
        for i in range(len(columns) - 1):
            lease += columns[i] + space * 2
            if i == 2:
                lease += space * (15 - len(columns[2]))
        message += '\n' + lease
    lease_file.close()
    message += '</code></pre></body></html>'
    return message

@app.route('/admin/writecard/<member_uid>/<droid_uid>', methods=['GET'])
def writeCard(member_uid, droid_uid):
    member = database.get_member(member_uid)
    droid = database.get_droid(droid_uid)
    output = droid['name'] + "," + droid_uid + "," + member['name'] + "," + member_uid + "," + member['badge_id']
    print(output)
    ser.write(output)
    return "Ok"

if __name__ == '__main__':
    if __debug__:
        print("Starting R2 course")
    database.db_init()
    # app.run(host='0.0.0.0', debug=__debug__, use_reloader=False, threaded=True)
    socketio.run(app, host='0.0.0.0', debug=__debug__, use_reloader=False)

