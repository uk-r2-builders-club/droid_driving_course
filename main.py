#!/usr/bin/python3
""" R2 Builder Obstacle Course """
from __future__ import print_function
import os
import time
import datetime
import urllib.request
import json
import ast
import configparser
from pathlib import Path
from collections import namedtuple
from future import standard_library
from flask import Flask, request, render_template
from flask_socketio import SocketIO, emit
import database 

configfile = str(Path.home()) + "/.r2_builders/course.ini"
config = configparser.SafeConfigParser({'api_key': 'null',
                                        'key': 'SECRET' })
config.read(configfile)

if not os.path.isfile(configfile):
    print("Config file does not exist")
    with open(configfile, 'wt') as configfile:
        config.write(configfile)

defaults = config.defaults()

Droid = namedtuple('Droid', 'droid_uid, rfid, member_uid, name, material, weight, transmitter_type')
Driver = namedtuple('Driver', 'member_uid, rfid, name, email')

api_key = defaults['api_key']

current_droid = Droid(droid_uid=0, rfid="none", member_uid=0, name="none", material="none", weight="none", transmitter_type="none")
current_member = Driver(member_uid=0, rfid="none", name="none", email="none")
current_run = 0

def get_member_details(did):
    """ Pull member details using API """


app = Flask(__name__, template_folder='templates')
app.config['key'] = defaults['key']
socketio = SocketIO(app)


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

@app.route('/display/<cmd>')
def display(cmd):
    global current_droid, current_member
    if request.method == 'GET':
        if cmd == 'results':
            return database.list_results()
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
            if current_run != 0:
                return json.dumps(database.current_run(current_run))
            else: 
                return "none"
        if cmd == 'list_gates':
            gates = json.dumps(database.list_gates())
            if __debug__:
                print("Gates: %s" % gates)
            return gates
        if cmd == 'current_gates':
            if current_run != 0:
                return json.dumps(database.list_penalties(current_run))
    return "Ok"

@app.route('/droid/<did>', methods=['GET'])
def droid_register(did):
    global current_droid
    if request.method == 'GET':
        data = json.dumps(database.get_droid(did))
        current_droid = json.loads(data, object_hook=lambda d: namedtuple('Droid', d.keys())(*d.values()))
        print(current_droid)
        print("Droid Registered: %s" % did)
        socketio.emit('my_response', {'data': 'Droid Registered'}, namespace='/comms')
        socketio.emit('reload_contender', {'data': 'reload contender'}, namespace='/comms')
    return "Ok"

@app.route('/member/<did>', methods=['GET'])
def member_register(did):
    global current_member
    if request.method == 'GET':
        data = json.dumps(database.get_member(did))
        current_member = json.loads(data, object_hook=lambda d: namedtuple('Driver', d.keys())(*d.values()))
        print(current_member)
        print("Driver Registered: %s" % did)
        socketio.emit('my_response', {'data': 'Driver Registered'}, namespace='/comms')
        socketio.emit('reload_contender', {'data': 'reload contender'}, namespace='/comms')
    return "Ok"

@app.route('/gate/<gid>/<value>', methods=['GET'])
def gate_trigger(gid, value):
    global current_run
    if request.method == 'GET':
        if value == 'FAIL':
            database.log_penalty(gid, current_run)
            socketio.emit('my_response', {'data': 'PENALTY!!!'}, namespace='/comms')
            socketio.emit('reload_gates', {'data': 'reload current'}, namespace='/comms')
        else:
            print("Gate passed")
        print("Gate ID: %s | %s" % (gid, value))
    return "Ok"

@app.route('/run/<cmd>', methods=['GET'])
def run_cmd(cmd):
    global current_run, current_member, current_droid
    if request.method == 'GET':
        if __debug__:
            print("Current run: %s | Current Driver: %s | Current Droid: %s " % (current_run, current_member.member_uid, current_droid.droid_uid))
        if cmd == 'START':
            current_run = database.run(0, cmd, current_member.member_uid, current_droid.droid_uid)
            socketio.emit('my_response', {'data': 'Start Run'}, namespace='/comms')
        if cmd == 'MIDDLE_WAIT':
            database.run(current_run, cmd, current_member.member_uid, current_droid.droid_uid)
            socketio.emit('my_response', {'data': 'Halfway Rest'}, namespace='/comms')
        if cmd == 'MIDDLE_START':
            database.run(current_run, cmd, current_member.member_uid, current_droid.droid_uid)
            socketio.emit('my_response', {'data': 'Continuing Run'}, namespace='/comms')
        if cmd == 'FINISH':
            database.run(current_run, cmd, current_member.member_uid, current_droid.droid_uid)
            socketio.emit('my_response', {'data': 'Finish!'}, namespace='/comms')
            socketio.emit('reload_results', {'data': 'reload results'}, namespace='/comms') 
        if cmd == 'RESET':
            current_run = 0
            current_droid = Droid(droid_uid=0, rfid="none", member_uid=0, name="none", material="none", weight="none", transmitter_type="none")
            current_member = Driver(member_uid=0, rfid="none", name="none", email="none")
            socketio.emit('my_response', {'data': 'Resetting'}, namespace='/comms')
            socketio.emit('reload_results', {'data': 'reload results'}, namespace='/comms')
            socketio.emit('reload_contender', {'data': 'reload contender'}, namespace='/comms')
            socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
            socketio.emit('reload_gates', {'data': 'reload current'}, namespace='/comms')
        socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
    return "Ok"

@app.route('/admin')
def admin():
    return render_template('admin.html', async_mode=socketio.async_mode)

@app.route('/admin/clear_db', methods=['GET'])
def clear_db():
    if request.method == 'GET':
        database.clear_db("all")
        socketio.emit('my_response', {'data': '**ADMIN** Database cleared'}, namespace='/comms')
        socketio.emit('reload_results', {'data': 'reload results'}, namespace='/comms')
    return "Ok"

@app.route('/admin/refresh/members', methods=['GET'])
def refresh_members():
    if request.method == 'GET':
        database.clear_db("members")
        uids = ast.literal_eval(urllib.request.urlopen("https://r2djp.co.uk/new_mot/api.php?api=" + api_key + "&request=list_member_uid").read().decode('utf-8').rstrip('\n'))
        if __debug__:
            print("UIDs: %s" % uids)
        for uid in uids:
           member = json.loads(urllib.request.urlopen("https://r2djp.co.uk/new_mot/api.php?api=" + api_key + "&request=member&id=" + uid).read())
           database.add_member(member)
           urllib.request.urlretrieve("https://r2djp.co.uk/new_mot/api.php?api=" + api_key + "&request=mug_shot&id=" + uid, "static/members/" + uid + ".jpg")
        socketio.emit('my_response', {'data': '**ADMIN** Members list refreshed'}, namespace='/comms')
    return "Ok"

@app.route('/admin/refresh/droids', methods=['GET'])
def refresh_droids():
    if request.method == 'GET':
        database.clear_db("droids")
        uids = ast.literal_eval(urllib.request.urlopen("https://r2djp.co.uk/new_mot/api.php?api=" + api_key + "&request=list_droid_uid").read().decode('utf-8').rstrip('\n'))
        if __debug__:
            print("UIDs: %s" % uids)
        for uid in uids:
           droid = json.loads(urllib.request.urlopen("https://r2djp.co.uk/new_mot/api.php?api=" + api_key + "&request=droid&id=" + uid).read())
           database.add_droid(droid)
           urllib.request.urlretrieve("https://r2djp.co.uk/new_mot/api.php?api=" + api_key + "&request=droid_shot&id=" + uid, "static/droids/" + uid + ".jpg")
        socketio.emit('my_response', {'data': '**ADMIN** Droids list refreshed'}, namespace='/comms')
    return "Ok"

@app.route('/admin/refresh/scoreboard', methods=['GET'])
def refresh_scoreboard():
    if request.method == 'GET':
        socketio.emit('my_response', {'data': '**ADMIN** Scoreboard refreshed'}, namespace='/comms')
        socketio.emit('reload_results', {'data': 'reload results'}, namespace='/comms')
        socketio.emit('reload_current', {'data': 'reload current'}, namespace='/comms')
        socketio.emit('reload_gates', {'data': 'reload current'}, namespace='/comms')
    return "Ok"

@app.route('/rfid/<tag>', methods=['GET'])
def read_rfid(tag):
    if request.method == 'GET':
        if __debug__:
            print("Received RFID tag: %s" % tag)
    return "Ok"

if __name__ == '__main__':
    database.db_init()
    # app.run(host='0.0.0.0', debug=__debug__, use_reloader=False, threaded=True)
    socketio.run(app, host='0.0.0.0', debug=__debug__, use_reloader=False)

