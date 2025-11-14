#!/usr/bin/env python3
# Copyright 2025 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import argparse
import requests
import sys

args = None
api = "http://127.0.0.1:8080"
session = requests.Session()


def parse_args():
    global args

    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(title="action", dest="action", required=True)

    subparser = subparsers.add_parser("add_default_apn")

    subparser = subparsers.add_parser("add_subscr")
    subparser.add_argument("--imsi", required=True)
    subparser.add_argument("--msisdn", required=True)
    subparser.add_argument("--auc-id", required=True)
    subparser.add_argument("--algo", required=True)
    subparser.add_argument("--ki", required=True)
    subparser.add_argument("--opc", required=True)

    subparser = subparsers.add_parser("del_subscr")
    subparser.add_argument("--imsi", required=True)

    args = parser.parse_args()


def add_default_apn():
    url = f"{api}/apn/"
    print(f"PUT {url}")
    payload = {
        "apn_id": 1,
        "apn": "internet",
        "ip_version": 0,
        "charging_characteristics": "0800",
        "apn_ambr_dl": 0,
        "apn_ambr_ul": 0,
        "qci": 9,
        "arp_priority": 4,
        "arp_preemption_capability": 0,
        "arp_preemption_vulnerability": 1,
    }
    session.put(url, json=payload)


def add_subscr():
    # Previous tests may have left an entry in the AUC table
    url = f"{api}/auc/{args.auc_id}"
    print(f"DELETE {url}")
    session.delete(url)

    url = f"{api}/auc/"
    print(f"PUT {url}")
    payload = {
        "auc_id": args.auc_id,
        "ki": args.ki,
        "opc": args.opc,
        "amf": "8000",
        "sqn": "0",
        "imsi": args.imsi,
        "algo": args.algo,
    }
    session.put(url, json=payload)

    url = f"{api}/subscriber/"
    print(f"PUT {url}")
    payload = {
        "auc_id": args.auc_id,
        "default_apn": "internet",
        "apn_list": "1",
        "imsi": args.imsi,
        "msisdn": args.msisdn,
    }
    session.put(url, json=payload)


def get_subscr_by_imsi():
    url = f"{api}/subscriber/imsi/{args.imsi}"
    print(f"GET {url}")
    ret = session.get(url).json()

    if not ret:
        print("ERROR: subscriber does not exist")
        sys.exit(1)

    return ret


def del_subscr():
    subscr = get_subscr_by_imsi()

    url = f"{api}/auc/{subscr['auc_id']}"
    print(f"DELETE {url}")
    session.delete(url)

    url = f"{api}/subscriber/{subscr['subscriber_id']}"
    print(f"DELETE {url}")
    session.delete(url)


if __name__ == "__main__":
    parse_args()
    globals()[args.action]()
