import requests
from urllib.parse import urlparse, parse_qs
import json
import sys

DEBUG = False
HOST = "http://localhost:8080/"


def main(args):

	if len(args) == 3:

		response = None

		method = args[0].strip().upper()
		path = args[1].strip()
		query = args[2].strip()

		if DEBUG:
			print( "{} {} {}\n".format( method, path, query ) )

		if method == "GET":
			if len(query) > 0:
				msg = HOST + path + "?" + query
			else:
				msg = HOST + path
			response = requests.get( msg )
		elif method == "POST":
			msg = HOST + path 
			response = requests.post( msg, data=query )

		if response is not None:
			print( "Recieved JSON {}".format( response.text ) )
			js_dict = json.loads( response.text )
			print( "Converted to dictionary{}:{}".format( type(js_dict), js_dict ) )
		else:
			print( "Unknown method (i.e., not GET or POST)" )
			print( "Usage: python request.py \"<method>\" \"<url>\"" )

	else:
		print("Usage: python request.py \"<method>\" \"<path>\" \"query\"\n")
		print("For futher details, see GitHub assignment.")

if __name__ == "__main__":
	args = sys.argv[1:]
	main(args)



