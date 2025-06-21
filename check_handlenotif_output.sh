#!/bin/bash
echo "Starting test suite and monitoring HandleNotification..."
cd /app/tt-smdpp
timeout 150 ./uns.sh 2>&1 | tee test_output.log | grep -E "(HandleNotification|TC_SM_DP_ES9_HandleNotificationNIST|pass|fail|error)"
echo "Test completed. Checking for HandleNotification in logs..."
grep -E "HandleNotification|TC_SM_DP_ES9_HandleNotificationNIST" test_output.log || echo "HandleNotification test not found in output"