#! /usr/bin/python3

import os
import sys
import asyncio
import json
import simpleobsws

loop = asyncio.get_event_loop()

_exitcode = 0

async def connect_and_send(rr):
	# NOTE: User has to edit host, port, and password.
	try:
		obsws_host = os.environ['OBSWS_HOST']
	except:
		obsws_host = '127.0.0.1'
	try:
		obsws_port = int(os.environ['OBSWS_PORT'])
	except:
		obsws_port = 4455
	try:
		obsws_passwd = os.environ['OBSWS_PASSWD']
	except:
		obsws_passwd = None
	ws = simpleobsws.WebSocketClient(url=f'ws://{obsws_host}:{obsws_port}', password=obsws_passwd)
	await ws.connect()
	await ws.wait_until_identified()
	for req, data in rr:
		if req=='sleep':
			await asyncio.sleep(data)
		else:
			req = simpleobsws.Request(req, data)
			res = await ws.call(req)
			if res.ok():
				print(json.dumps(res.responseData, indent="\t"))
			else:
				global _exitcode
				_exitcode = 1
				print('Error: %s' % str(res.responseData), file=sys.stderr)
	await ws.disconnect()

help_str='''NAME
	request-websocket.py - a helper command to send requests to OBS Studio

SYNOPSIS
	request-websocket.py [request_type data] ...
	request-websocket.py --help

DESCRIPTION
	This script will take pairs of request_type and data and send each pair to OBS Studio.
	The last pair can skip data if you don't need to send any data.

	request_type
		This is a request-type supported by obs-websocket protocol or 'sleep'.
		If request_type is 'sleep', data has to be a non-negative number to sleep in seconds.
		Otherwise, the request-type will be directly send to obs-websocket.
	data
		The data is a JSON text. The data can be an empty string if you don't need to send any data.

EXAMPLES
	request-websocket.py GetVersion
	request-websocket.py SetPreviewScene '{"scene-name": "Scene"}'
	request-websocket.py SetCurrentScene '{"scene-name": "Scene1"}' sleep 10 SetCurrentScene '{"scene-name": "Scene2"}'
'''

def main():
	import sys
	req = None
	rr = list()
	st = 0
	for a in sys.argv[1:]:
		if a=='--help':
			print(help_str)
			return 0
		if st==0:
			req = a
			st = 1
		elif st==1:
			if len(a):
				data = json.loads(a)
			else:
				data = None
			rr.append((req, data))
			st = 0
	if st==1:
		rr.append((req, None))
	loop.run_until_complete(connect_and_send(rr))

if __name__ == '__main__':
	main()
	sys.exit(_exitcode)
