#!/usr/bin/env bash
#
# Inspired by http://me-ol-blog.blogspot.ru/2017/07/getting-still-image-urluri-of-ipcam-or.html

ip=$1
port=$2
url="http://${ip}:${port}/"

curl -H "Content-Type: text/xml" -d @- -X POST ${url} <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<soap:Envelope xmlns:soap="http://www.w3.org/2003/05/soap-envelope"
xmlns:trt="http://www.onvif.org/ver10/media/wsdl"
xmlns:tt="http://www.onvif.org/ver10/schema">
  <soap:Body>
    <trt:GetStreamUri>
      <trt:StreamSetup>
        <tt:Stream>RTP-Unicast</tt:Stream>
          <tt:Transport>
            <tt:Protocol>UDP</tt:Protocol>
          </Transport>
        </trt:StreamSetup>
      <trt:ProfileToken>000</trt:ProfileToken>
    </trt:GetStreamUri>
  </soap:Body>
</soap:Envelope>
EOF
