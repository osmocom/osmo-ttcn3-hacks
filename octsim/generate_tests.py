#!/usr/bin/env python3

slots = range(0, 8, 1)
volts = ["CCID_PWRSEL_1V8", "CCID_PWRSEL_3V0", "CCID_PWRSEL_5V0"]


text = """testcase TC_test_slot{slot}_{volt}() runs on test_CT
{{
	f_init({slot});
	TC_test_common({slot}, {volt}, {pps});
	f_ccid_power_off(?);
}}
"""

text2 = """
control {
"""
text3 = """execute( TC_test_slot{slot}_{volt}() );"""
text4 = """
}
"""


for s in slots:
    for i in volts:  # range(0,10,1):
        pps = "false" if i == "CCID_PWRSEL_5V0" else "true"
        print(text.format(slot=s, volt=i, pps=pps))

print(text2)
for s in slots:
    for i in volts:  # range(0,10,1):
        pps = "false" if i == "CCID_PWRSEL_5V0" else "true"
        print(text3.format(slot=s, volt=i, pps=pps))
print(text4)
