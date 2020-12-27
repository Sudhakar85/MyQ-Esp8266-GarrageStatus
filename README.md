# MyQ - Check garrage status using MyQ API in ESP8266 - Wemos D1


1. Update WIFI SSID and pwd       
2. Update MyQ username and password
3. Update the AccountId


##Reference MyQ Endpoint
1. POST - https://api.myqdevice.com/api/v5/login - Get Auth Token by passing MyQ user id and pwd
2. GET - https://api.myqdevice.com/api/v5/my?expand=account -pass the auth token and get the account Id
3. GET - http://api.myqdevice.com/api/v5.1/Accounts/<AccountId>/Devices - Pass the account id and get the connected device status
