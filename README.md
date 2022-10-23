# kore.io.raise3d

https proxy to raise3d API

# Enable
To enable Remote Access API on your Raise3D Printer follow this 
https://support.raise3d.com/RaiseTouch/6-1-7-1-enable-remote-access-api-17-895.html

# Kore.io
To run it follow kore.io help.

# Configure
First edit config file raise-conf.json, and complete it.

```JSON
{
    "raise_base_urlb" : "http://W.X.Y.Z:10800/v1/" ,
    "raise_passwd" : "PASSWORD"
}
```

# Test
backend expose 4 end points

## left nozzle
return left nozzle state

```bash
$ curl -i -k -X GET  https://127.0.0.1:8888/left-nozzle
HTTP/1.1 200 OK
server: kore (master-23d762d6)
connection: keep-alive
keep-alive: timeout=20
strict-transport-security: max-age=31536000; includeSubDomains
Content-Type: application/json
content-length: 102
```

```json
{
  "data": {
    "flow_cur_rate": 98,
    "flow_tar_rate": 98,
    "nozzle_cur_temp": 276,
    "nozzle_tar_temp": 275
  },
  "status": 1
}
```

## right nozzle
return right nozzle state

```bash
$ curl -i -k -X GET  https://127.0.0.1:8888/right-nozzle
HTTP/1.1 200 OK
server: kore (master-23d762d6)
connection: keep-alive
keep-alive: timeout=20
strict-transport-security: max-age=31536000; includeSubDomains
Content-Type: application/json
content-length: 102
```

```json
{
  "data": {
    "flow_cur_rate": 100,
    "flow_tar_rate": 100,
    "nozzle_cur_temp": 276,
    "nozzle_tar_temp": 275
  },
  "status": 1
}
```

## printer-basics
return printer basics

```bash
$ curl -i -k -X GET  https://127.0.0.1:8888/printer-basics
HTTP/1.1 200 OK
server: kore (master-23d762d6)
connection: keep-alive
keep-alive: timeout=20
strict-transport-security: max-age=31536000; includeSubDomains
Content-Type: application/json
content-length: 142
```

```json
{
  "data": {
    "fan_cur_speed": 50,
    "fan_tar_speed": 50,
    "feed_cur_rate": 100,
    "feed_tar_rate": 100,
    "heatbed_cur_temp": 99,
    "heatbed_tar_temp": 100
  },
  "status": 1
}
```

## job-status
return current job status

```bash
$ curl -i -k -X GET  https://127.0.0.1:8888/job-status
HTTP/1.1 200 OK
server: kore (master-23d762d6)
connection: keep-alive
keep-alive: timeout=20
strict-transport-security: max-age=31536000; includeSubDomains
Content-Type: application/json
content-length: 239
```

```json
{
  "data": {
    "file_name": "Local/medaille_1.gcode",
    "job_id": "C400AD844745_48E7DAEB839B_7e60a1715281a13a10",
    "job_status": "running",
    "print_progress": 1.1019070148468018,
    "printed_layer": 6,
    "printed_time": 444,
    "total_layer": 43,
    "total_time": 3689
  },
  "status": 1
}
```
