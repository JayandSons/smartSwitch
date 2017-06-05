import httplib

conn = httplib.HTTPConnection("192.168.4.1")

#Sending the name of the house WiFi
conn.request("GET", "/MyResNet")
r1 = conn.getresponse()
data = r1.read()
print data

#Sending the password of the house WiFi
conn.request("GET", "/MyResNetPassword")
r1 = conn.getresponse()
data = r1.read()
print data

#Requesting the MAC ID of the chip
conn.request("GET", "/MyResNetPassword")
r1 = conn.getresponse()
data = r1.read()
print data

conn.close()