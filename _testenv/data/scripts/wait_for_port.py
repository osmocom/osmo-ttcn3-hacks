#!/usr/bin/env python3
# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
# Wait until a port is available, useful for waiting until a SUT has started
import socket
import argparse
import time
import sys

args = None


def wait_for_port():
    start_time = time.time()
    while True:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            result = s.connect_ex((args.hostname, args.port))
            if result == 0:
                sys.exit(0)

            if time.time() - start_time >= args.timeout:
                print(f"ERROR: {args.hostname}:{args.port} did not become available within {args.timeout}s!")
                sys.exit(1)

            time.sleep(0.1)


def parse_args():
    global args

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-H",
        "--hostname",
        help="default: 127.0.0.1",
        default="127.0.0.1",
    )
    parser.add_argument(
        "-p",
        "--port",
        type=int,
        required=True,
    )
    parser.add_argument(
        "-t",
        "--timeout",
        type=int,
        default=5,
        help="timeout in seconds (default: 5).",
    )
    args = parser.parse_args()


if __name__ == "__main__":
    parse_args()
    wait_for_port()
