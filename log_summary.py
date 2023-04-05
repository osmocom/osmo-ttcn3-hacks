#!/usr/bin/env python3

import sys
import re

class Component:
	components = {}

	def __init__(s, nr, name=None, t=None):
		s.nr = nr
		s.name = name
		s.t = t
		s.alias = None
		Component.components[nr] = s

	@classmethod
	def get(cls, nr):
		component = Component.components.get(nr)
		if component is None:
			component = Component(nr)
		return component

	@classmethod
	def set(cls, nr, name=None, t=None):
		component = Component.get(nr)
		if name is not None:
			component.name = name
		if t is not None:
			component.t = t
		component.set_alias()
		return component

	alias_idx = {}
	aliases = {
		'MSC_ConnectionHandler.MSC_ConnHdlr' : 'msc_conn',
		'RSL_Emulation.RSL_Emulation_CT' : 'rsl',
		'RAN_Emulation.RAN_Emulation_CT' : 'ran',
		'IPA_Emulation.IPA_Emulation_CT' : 'ipa',
		}

	def set_alias(s):
		alias = None
		add_nr = True
		if s.t == 'IPA_Emulation.IPA_Emulation_CT':
			if s.name == 'IPA-CTRL-CLI-IPA':
				alias = 'ctrl'
			elif s.name.startswith('IPA-BTS'):
				alias = re.findall('BTS[0-9]+-TRX[0-9]+', s.name)[0]
				add_nr = False

		if not alias:
			alias = Component.aliases.get(s.t)

		if alias:
			created = False
			seen = Component.alias_idx.get(alias)
			if seen is None:
				seen = []
				Component.alias_idx[alias] = seen
			if s.nr not in seen:
				seen.append(s.nr)
				created = True
			idx = seen.index(s.nr)
			if add_nr:
				s.alias = f'{alias}{idx}'
			else:
				s.alias = alias
			if created:
				print('* ' + repr(s))
		return s.alias

	def __str__(s):
		if s.alias:
			return s.alias
		return ' '.join(str(x) for x in (s.nr, s.name, s.t) if x is not None)

	def __repr__(s):
		return ' '.join(str(x) for x in (s.alias + ' =' if s.alias else None, s.nr, s.name, s.t) if x is not None)

class Message:
	messages = {}

	def __init__(s, component, nr, msgt, data):
		s.rx = None
		s.tx = None
		s.component = Component.get(component)
		s.nr = nr
		s.msgt = msgt
		s.data = data
		if nr is not None:
			Message.messages[nr] = s

	@classmethod
	def get(cls, nr):
		return Message.messages.get(nr)

	@classmethod
	def rx(cls, port, component, nr, msgt, data=None):
		created = False
		msg = Message.get(nr)
		if msg is None:
			msg = Message(component, nr, msgt, data)
			created = True
		if msgt is not None:
			msg.msgt = msgt
		if data is not None:
			msg.data = data
		msg.rx = port
		if created:
			print(repr(msg))
		return msg

	def __repr__(s):
		i = []
		if s.component:
			i.append(s.component)
		if s.rx:
			i.append(f'> {s.rx}')
		if s.tx:
			i.append(f'< {s.tx}')
		if s.msgt:
			i.append(s.msgt)
		return ' '.join(str(x) for x in i)




# Starting function main("127.0.0.1", 8125) on component VirtMSC-STATS(3).
re_fun = re.compile('Starting function ([^(]+)\(.*\) on component ([^(]+)\(([0-9]+)\)\.')
re_prefix = re.compile('([0-9:.]+) [^ ]+ ([A-Za-z_-]+\.ttcn[p]*):([0-9]+) (.*)')
re_prefix2 = re.compile('([0-9:.]+) ([0-9]+|mtc) - (.*)')
re_prefix3 = re.compile('([0-9:.]+) ([0-9]+|mtc) ([A-Za-z0-9_-]+\.ttcn[p]*):([0-9]+) (.*)')
re_tc = re.compile('Test case (.*) started')

# PTC was created. Component reference: 5, alive: no, type: RAN_Emulation.RAN_Emulation_CT, component name: VirtMSC-RAN.
re_ptc = re.compile('PTC was created. Component reference: ([0-9]+),.* type: (.*), component name: (.*)\.')

# Receive operation on port SCTP_PORT succeeded, message from system(): @SCTPasp_Types.ASP_SCTP : { client_id := 7, sinfo_stream := 0, sinfo_ppid := 3, data := '0100030400000008'O } id 3
#  Receive operation on port IPA_CTRL succeeded, message from IPA-CTRL-CLI-IPA(13): @IPA_Emulation.ASP_IPA_Event : { ev_type := ASP_IPA_EVENT_UP (1), conn_id := 1, id_resp := omit } id 1
re_rx = re.compile('Receive operation on port (.*) succeeded, message from ([^(]+)\(([0-9]+)\): ([^ ]+) *: (.+) id ([0-9]+)')
re_rx2 = re.compile('Receive operation on port (.*) succeeded, message from ([^: ]+): ([^ ]+) *: (.+) id ([0-9]+)')

# Sent on IPA_CTRL to IPA-CTRL-CLI-IPA(13) @Osmocom_CTRL_Types.CtrlMessage : { cmd := { verb := "GET", id := "332724832", variable := "rate_ctr.abs.msc.0.mscpool:subscr:reattach", val := omit } }
re_tx = re.compile('Sent on (.+) to ([^(]+)\(([0-9]+)\) ([^:]+) *: (.+)')
# Sent on IPA_PORT to system @IPA_CodecPort.IPA_Send : { connId := 1, streamId := IPAC_PROTO_OSMO (238), streamIdExt := IPAC_PROTO_EXT_CTRL (0), msg := '4745542031383934323239333220726174655F6374722E6162732E6D73632E302E6D7363706F6F6C3A7375627363723A6E6577'O ("GET 189422932 rate_ctr.abs.msc.0.mscpool:subscr:new") }
re_tx2 = re.compile('Sent on (.+) to ([^ ]+) ([^:]+) *: (.+)')

# Dynamic test case error: Port BSSAP_PROC has neither connections nor mappings. Message cannot be sent on it.
re_dynerr = re.compile('(Dynamic test case error: .*)')

# setverdict(error): none -> error
re_verdict = re.compile('setverdict\(([^)]+)\): ([^ ]+) .*')

for line in open(sys.argv[1]):
	time = None
	srcfile = None
	srcline = None
	component = None
	component = None
	msg = None

	m = re_prefix.match(line)
	if m:
		time, srcfile, srcline, msg = m.groups()
	else:
		m = re_prefix2.match(line)
		if m:
			time, component, msg = m.groups()
		else:
			m = re_prefix3.match(line)
			if m:
				time, component, srcfile, srcline, msg = m.groups()

		if False and not m:
			print("NOT " + line)
			if line.find('BSC_Tests') >= 0:
				sys.exit(1)
			continue

	if component is not None:
		component = Component.get(component)

	show = []

	if msg is None:
		continue

	m = re_tc.match(msg)
	if m:
		tc = m.group(1)
		show.append(f'{srcfile} {tc}()')

	m = re_fun.match(msg)
	if m:
		fun, component_name, component = m.groups()
		component = Component.set(component, component_name)
		show.append(f'{fun}()')

	m = re_ptc.match(msg)
	if m:
		component, component_type, component_name = m.groups()
		component = Component.set(component, component_name, component_type)

	m = re_rx.match(msg)
	if m:
		port, from_component_name, from_component, msgtype, msgdata, msgid = m.groups()
		Message.rx(port, from_component, msgid, msgtype, msgdata)
	else:
		m = re_rx2.match(msg)
		if m:
			port, from_component_name, msgtype, msgdata, msgid = m.groups()
			Message.rx(port, from_component_name, msgid, msgtype, msgdata)
		elif msg.find('Receive operation') >= 0:
			print("XXX ", msg)
			sys.exit(0)

	m = re_tx.match(msg)
	if m:
		port, to_component_name, to_component_nr, msgtype, msgdata = m.groups()
		to_cmp = Component.get(to_component_nr)
		show.append(f'{to_cmp} < {port} {msgtype}')

	else:
		m = re_tx2.match(msg)
		if m:
			port, to_component, msgtype, msgdata = m.groups()
			show.append(f'{to_component} < {port} {msgtype}')
		elif 'Sent on' in msg:
			print('XXX ', msg)
			sys.exit(0)

	m = re_dynerr.match(msg)
	if m:
		show.append(m.group(0))

	m = re_verdict.match(msg)
	if m:
		to_verdict, from_verdict = m.groups()
		if to_verdict != 'pass':
			show.append(f'***** verdict: {to_verdict}')

	ctx = []
	if component is not None:
		ctx.append(f'{component}')
	if srcfile:
		ctx.append(f'{srcfile}:{srcline}')

	for s in show:
		print(f'{s}  ' + ' '.join(ctx))



