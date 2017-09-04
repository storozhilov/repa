#!/usr/bin/env bash
#
# Inspired by http://me-ol-blog.blogspot.ru/2017/07/getting-still-image-urluri-of-ipcam-or.html

ip=$1
port=$2
url="http://${ip}:${port}/"

curl -H "Content-Type: text/xml" -d @- -X POST ${url} <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://www.w3.org/2003/05/soap-envelope" xmlns:trt="http://www.onvif.org/ver10/media/wsdl">
  <SOAP-ENV:Body>
    <trt:GetProfiles/>
  </SOAP-ENV:Body>
</SOAP-ENV:Envelope>
EOF
