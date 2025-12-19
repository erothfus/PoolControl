#
# Makefile
#

tunnel:
	ssh -N -R 0.0.0.0:30123:localhost:8080 eric@rothfus.com

sshtunnel:
	ssh -N -R localhost:30124:localhost:22 eric@rothfus.com
