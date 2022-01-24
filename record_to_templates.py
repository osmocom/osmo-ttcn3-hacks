#!/usr/bin/env python3
import re
import sys

re_record_start = re.compile(r'^type +(set|record) +([A-Za-z_][A-Za-z0-9_]*) *{$')
re_record_end = re.compile(r'^}.*')
re_member_preset = re.compile(r'[ \t]*([^ \t].*)[ \t]+([a-zA-Z_][A-Za-z0-9_]*)[ \t]*\(([^)]+)\)[ \t]*,[ \t]*')
re_member_mandatory = re.compile(r'[ \t]*([^ \t].*)[ \t]+([a-zA-Z_][A-Za-z0-9_]*)[ \t]*,[ \t]*')
re_member_optional = re.compile(r'[ \t]*([^ \t].*)[ \t]+([a-zA-Z_][A-Za-z0-9_]*)[ \t]+optional[ \t]*,[ \t]*')
re_comment_1line = re.compile(r'[ \t]*//.*')

def parse_record(name, lines):
	for i in range(len(lines)):
		lines[i] = re_comment_1line.sub('', lines[i])
	lines[-1] = lines[-1].strip() + ','
	members = []
	for line in lines:
		optional = False
		preset = None
		m = re_member_preset.match(line)
		if m:
			preset = m.group(3)
		else:
			optional = True
			m = re_member_optional.match(line)
			if not m:
				optional = False
				m = re_member_mandatory.match(line)
			if not m:
				continue
		member_type = m.group(1)
		member_name = m.group(2)
		members.append((member_type, member_name, optional, preset))

	return members

def run_record(name, lines):
	if not lines:
		return
	members = parse_record(name, lines)
	print_tr(name, members)
	print_ts(name, members)

proto = 'PFCP'
template_type = 'PDU_PFCP'

def print_tr(name, members):
	print('')
	print(f'template {template_type} tr_{proto}_{name} := {{')
	for i in range(len(members)):
		member_type, member_name, optional, preset = members[i]
		last = (i == len(members) - 1)
		if preset:
			optstr = preset
		else:
			optstr = '*' if optional else '?'
		comma = '' if last else ','
		print(f'\t{member_name} := {optstr}{comma}')
	print('}')

def print_ts(name, members):
	print('')
	print(f'template {template_type} ts_{proto}_{name} := {{')
	for i in range(len(members)):
		member_type, member_name, optional, preset = members[i]
		last = (i == len(members) - 1)
		if preset:
			optstr = preset
		else:
			optstr = 'omit' if optional else '0'
		comma = '' if last else ','
		print(f'\t{member_name} := {optstr}{comma}')
	print('}')


read_record_name = None
read_record = []

for line in sys.stdin.readlines():

	if read_record_name:
		if re_record_end.match(line):
			run_record(read_record_name, read_record)
			read_record_name = None
			read_record = []
		else:
			read_record.append(line)
		continue

	m = re_record_start.match(line)
	if m:
		read_record_name = m.group(2)
