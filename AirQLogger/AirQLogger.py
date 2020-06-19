#!/usr/bin/env python3
# Copyright (C) 2020 Martin Weigel <mail@MartinWeigel.com>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
import argparse
import msgpack
import socket
import time

# Parse arguments
parser = argparse.ArgumentParser(description='Receives UDP packets of AirQ and saves sensor data into a csv file.')
parser.add_argument('ip', type=str,
    help='The network IP of the server to listens for UDP packets')
parser.add_argument('--port', type=int, default=5777,
    help='Port to listen for UDP packets (default: 5777)')
parser.add_argument('--log', type=str, default='airq-log.csv',
    help='File to log data to (default: airq-log.csv)')
args = parser.parse_args()

# Create socket listener
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((args.ip, args.port))
print("Started UDP Logger ({}:{})".format(args.ip, args.port))

while True:
    # Receive data over socket (maximum message size is 1024 bytes)
    data, addr = sock.recvfrom(1024)
    print("{}: Message from {}".format(time.time(), addr))

    # Save data as csv to file
    unpacked = msgpack.unpackb(data, raw=False)
    with open(args.log, 'a') as f:
        f.write("{}\t{}\t{}\t{}\t{}\t{}\n".format(
            unpacked['name'],
            unpacked['time'],
            unpacked['temperature'],
            unpacked['pressure'],
            unpacked['humidity'],
            unpacked['gas']))
