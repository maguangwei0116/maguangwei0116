
adb push .\app_targets\rt_agent /usr/bin/rt_agent
adb shell chmod +x /usr/bin/rt_agent

adb push .\app_targets\rt_monitor /usr/bin/rt_monitor
adb shell chmod +x /usr/bin/rt_monitor

adb push .\app_targets\libcomm.so /usr/lib/libcomm.so
adb shell chmod +x /usr/lib/libcomm.so

if exist .\app_targets\rt_share_profile.der  (
	adb push .\app_targets\rt_share_profile.der /data/redtea/rt_share_profile.der
	adb shell chmod +x /data/redtea/rt_share_profile.der
)

adb push .\app_targets\libskb.so /usr/lib/libskb.so
adb shell chmod +x /usr/lib/libskb.so

adb push .\app_shells\start_redtea_app /etc/init.d/start_redtea_app
adb shell chmod +x /etc/init.d/start_redtea_app

adb push .\app_shells\start_redtea_keep /etc/init.d/start_redtea_keep
adb shell chmod +x /etc/init.d/start_redtea_keep

if exist .\app_targets\test_lpa (
	adb push .\app_targets\test_lpa /data/redtea/test_lpa
	adb shell chmod +x /data/redtea/test_lpa
)

if exist .\app_targets\test_client (
	adb push .\app_targets\test_client /data/redtea/test_client
	adb shell chmod +x /data/redtea/test_client
)

adb shell rm -rf /etc/rc5.d/S99start_redtea_app
adb shell ln -s /etc/init.d/start_redtea_app /etc/rc5.d/S99start_redtea_app
