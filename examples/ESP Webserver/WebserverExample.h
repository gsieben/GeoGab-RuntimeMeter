#ifndef WebserverExample_h
#define WebserverExample_h

#include <Arduino.h>
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include <ArduinoJson.h> 

#include <GeoGabRuntimeMeter.h>

/* Prototypes */
void JsonAnswer();
void SendIndex();

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html> <html> <head> <meta charset="utf-8"> <title>GeoGab Home Devices</title> <link rel="icon" type="image/png" href="GeoGab-ico.png" sizes="32x32"> <style>body{font-family:Helvetica;font-size:small;font-weight:400;margin:0 auto;background-color:#f0eee6;width:1000px;overflow-y:scroll}header{position:sticky;top:0;background-color:#f0eee6;padding:0;z-index:99}main{background-color:#fff;padding:15px 15px 15px;margin:0 0 15px}h1,h2{font-size:20px;text-align:center;margin:0}h2{font-size:15px;padding-bottom:20px}.logobox{height:100px;width:100%}.page75{margin:0 auto;width:75%;text-align:center;box-shadow:10px 10px 5px 0 rgba(0,0,0,.26);padding:10px;border:2px solid #3333;background-color:#e7e7e9}#FileModeButton{width:200px;position:absolute;margin:0 auto;animation:filemod 1s infinite alternate linear}@keyframes filemod{from{background-color:#f0eee6}to{background-color:#8f2100}}table{width:100%}table{table-layout:fixed;margin:0 auto;background-color:transparent;border-spacing:3px 3px}td,th{background-color:#e7e7e9;border:2px solid #3333;box-shadow:10px 10px 5px 0 rgba(0,0,0,.26);padding:4px}.bar{display:inline-block;width:98%;box-shadow:10px 10px 5px 0 rgba(0,0,0,.26)}#p0,#p1,#p2,#p3{display:inline-block;width:0}#b0,#p0{background-color:#a9af6a}#b1,#p1{background-color:#fabf01}#b2,#p2{background-color:#eb85bc}#b3,#p3{background-color:#b51F5E}.s1{width:30%}.s2{width:10%}.s3{width:20%}</style><script>var DevIp;
		var burl;
		var loc = false;
 
		function OnLoad(){
			FileMode();		// File Mode for the Development
			LiveUpdate();
			!loc && window.setInterval(LiveUpdate, 1000);		// pull the data every second	
		}

		function FileMode() {
			if (window.location.protocol == "file:") {
				loc = true;
				var buffer=""
				locip = localStorage.getItem('locIp');
					buffer+="<b>Development Mode:</b><br><button onclick='NewIP()'>Set new IP</button>"
				if (typeof LiveUpdate === 'function') {     // Is function LiveUpdate() there? -> Add Manual Refresh Button
					buffer+="</b><button onclick=' LiveUpdate()'>Manual Refresh</button>"
				}
				if (!locip){
					locip = prompt("Development Mode: Please enter the device IP-Adress (RuntimeMeter-Example-Code)!");
					localStorage.setItem('locIp', locip);
				}
				document.getElementById("FileModeButton").innerHTML=buffer;
			}
		}

		function LiveUpdate(){
			url = loc ? `http://${locip}/json`:`/json`;
			fetch(url)
			.then( (resp) => {return resp.json();})
			.then( (obj) => {						
				// Create Table
				document.getElementById("CF").innerHTML=`${obj.CPUf} MHz`;
				let buff="<table><tr><th class='s1'>Name</th><th class='s2'>Legende</th><th class='s3'>Runtime</th><th class='s4'>Percentage</th></tr>";
				let OverallRT=obj.Runtime;
				let totalrt=0;
				for(var i=0;i<obj.no;i++) {
					let per=(obj.slots[i].Runtime/OverallRT*100);
					buff+=`<tr><td> ${obj.slots[i].Name}</td><td id='b${i}'></td><td>${obj.slots[i].Runtime}</td><td>${per.toFixed(2)} %</td></tr>`; 
					document.getElementById(`p${i}`).innerHTML=per.toFixed(2);
					document.getElementById(`p${i}`).style.width=`${per}%`
					totalrt+=obj.slots[i].Runtime;
				}

				buff+=`<tr><td>Overall</td><td></td><td>${totalrt}</td><td>100 %</td></tr>`; 
				buff+="</table>";
				document.getElementById("LF").innerHTML=`${(obj.CPUf*1000000/totalrt).toFixed(2)} Hz`;
				document.getElementById("list").innerHTML=buff;
				document.getElementById("source").href=url; 
		
			})
		}

		function NewIP() {
			localStorage.removeItem('locIp');
			FileMode();
		}</script></head><body onload="OnLoad()"> <header> <div id="FileModeButton"></div> <svg class="logobox"><use xlink:href="#GeoGab"/></svg> </header> <main> <div class="page75"> <h1>Runtime Meter Example Code</h1> <h2>by Gabriel A. Sieben</h2> <div>CPU Frequency:<span id="CF"></span></div> <div>Loop Frequency:<span id="LF"></span></div> <a id="source" href="" target="_blank">Json Data Source</a> <br><br> <div class="bar"><div id="p0"></div><div id="p1"></div><div id="p2"></div><div id="p3"></div></div> <br><br> <div id="list">loading...</div> </div> </main> </body> <span style="display:none">  <svg viewBox="0 0 600 140" id="GeoGab" width="1000px"> <defs> <filter id="dropshadow" height="130%"> <fegaussianblur in="SourceAlpha" stddeviation="8"/> <feoffset dx="4" dy="4" result="offsetblur"/> <femerge> <femergenode/> <femergenode in="SourceGraphic"/> </femerge> </filter> </defs> <path id="geogabbg" filter="url(#dropshadow)" style="fill:#fff" d="M 600,103.51782 C 600,116.79902 588.8778,127.56 575.17384,127.56 H 24.826163 C 11.111876,127.56 5.1635108e-7,116.79902 5.1635108e-7,103.51782 V 41.602186  C 5.1635108e-7,28.320979 11.111876,17.560001 24.826163,17.560001 H 575.16351 c 13.71429,0 24.82616,10.760978 24.82616,24.042185 v 61.915634 z"/> <path style="fill:#808285" d="M 600,103.51782 C 600,116.79902 588.8778,127.56 575.17384,127.56 H 24.826162 C 11.111877,127.56 5.1635111e-7,116.79902 5.1635111e-7,103.51782 V 41.602187 C  5.1635111e-7,28.320977 11.111876,17.559997 24.826162,17.559997 H 575.16351 c 13.71429,0 24.82616,10.76098 24.82616,24.04219 V 103.51782 Z M 157.07401,78.885567  c 0,17.15156 12.06196,19.07173 22.60585,19.07173 h 14.69535 c 15.07745,0 24.20654,-3.20029 24.20654,-13.87125 v -0.91009 h -8.09638 c -0.46472,7.11065 -4.61618, 8.39076 -14.7883,8.39076 h -11.11188 c -14.12736,0 -19.40447,-2.74024 -19.40447,-13.3212 v -2.37022 h 53.40103 v -5.75052 c 0,-16.60151 -10.45094,-20.16183  -24.96041,-20.16183 h -13.00172 c -11.48365,0 -23.54561,1.92017 -23.54561,18.79171 z m 53.41136,-9.9509 h -45.30465 c 0.75387,-10.58096 3.76936,-12.59114 16.57487, -12.59114 h 11.86575 c 13.18761,0 16.86403,4.74043 16.86403,9.49086 z m 19.22891,10.13092 c 0,15.33139 11.21515,18.88171 26.57143,18.88171 h 11.58692 c 15.35629,0  26.56111,-3.56032 26.56111,-18.88171 v -10.22093 c 0,-15.32139 -11.20482,-18.88171 -26.56111,-18.88171 h -11.58692 c -15.35628,0 -26.57143,3.56032 -26.57143,18.88171  z m 56.61275,-0.46004 c 0,10.86098 -6.58865,12.95117 -16.01722,12.95117 h -16.48193 c -9.41825,0 -16.01721,-2.09019 -16.01721,-12.95117 v -9.31085 c 0,-10.85098  6.59896,-12.95117 16.01721,-12.95117 h 16.48193 c 9.41824,0 16.01722,2.10019 16.01722,12.95117 z m 179.47332,18.61169 v -31.20283 c 0,-13.87126 -10.17212,-16.06146  -25.24957,-16.06146 h -12.81584 c -14.50946,0 -20.0654,3.47031 -20.0654,11.95108 v 2.28021 h 8.09638 v -1.64015 c 0,-3.83035 1.98279,-6.20056 12.15491,-6.20056 h  11.86575 c 12.43373,0 17.89673,1.0901 17.89673,9.49086 v 8.7608 h -0.18589 c -2.8296,-4.47041 -6.59897,-5.56051 -17.14286,-5.56051 h -13.00172 c -14.12736,0 -21.85198, 1.28012 -21.85198,11.86108 v 3.10028 c 0,8.76079 3.20138,13.96126 15.44923,13.96126 h 19.40447 z m -8.09639,-12.50114 c 0,5.84053 -5.64888,6.84062 -20.34423,6.84062  h -10.74011 c -11.11187,0 -13.00172,-2.19019 -13.00172,-7.39067 v -1.46013 c 0,-5.84053 2.92255,-7.30066 13.3735,-7.30066 h 12.43373 c 12.43374,0 18.27883,0.82007  18.27883,7.20065 z m -389.27711,-8.8008 h 11.2358 l 5.194492,0.01 h 50.261618 v 2.41021 c 0,9.12083 -5.64888,12.23111 -14.88124,12.23111 H 84.154905 c -8.860585,0  -16.017212,-2.55023 -16.017212,-14.05127 v -0.35003 -0.26003 z m -9.459552,-0.01 c 0,0.06 0,0.14002 0,0.20002 0.69191,14.68133 6.898451,21.76198 24.237521,21.76198  h 38.055071 c 17.04992,0 23.0809,-6.39058 23.0809,-24.36221 v -4.88045 H 84.857142 l -4.254733,-0.01 H 68.137693 v -0.01 -16.90153 c 0,-11.49104 7.156627,-14.05129  16.017212,-14.05129 h 23.927705 c 22.79174,0 26.65405,0.91009 26.65405,13.23121 h 8.66437 v -2.37021 c 0,-15.23139 -13.09466,-18.15166 -25.34251,-18.15166 H 83.215146  c -18.464717,0 -24.299483,8.03073 -24.299483,24.72226 v 13.53122 l -0.01033,0.01 h -20.42685 c -12.247849,0 -25.342513,2.92027 -25.342513,18.15165 v 2.37022 5.02045  2.37022 c 0,15.241393 13.094664,18.161653 25.342513,18.161653 h 42.123923 446.736664 35.14286 c 18.46471,0 24.30981,-8.03073 24.30981,-24.722253 v -34.87316 c 0, -16.69153 -5.83477,-24.72226 -24.30981,-24.72226 l -0.16523,-0.02 H 332.0241 c -18.46471,0 -24.29948,8.03073 -24.29948,24.72226 v 18.06163 c 0,16.69152 5.83477,24.72225  24.29948,24.72225 h 38.05508 c 17.04991,0 23.08089,-6.39058 23.08089,-24.36221 v -4.88045 h -44.179 v 7.30067 h 34.94664 v 2.41021 c 0,9.12083 -5.64888,12.23111  -14.88123,12.23111 h -36.08262 c -8.86059,0 -16.00688,-2.55023 -16.00688,-14.05127 v -24.81225 c 0,-11.49104 7.15662,-14.05129 16.00688,-14.05129 l 147.16007,0.02  v 61.24556 l 25.14629,0.73007 h 11.30809 c 14.69536,0 24.97074,-3.56033 24.97074,-19.61178 v -8.21075 c 0,-14.87135 -5.93803,-20.16183 -25.43545,-20.16183 h -12.34079  -15.36661 -0.17556 l -0.0207,-14.01128 h 74.09639 l -0.77453,0.02 c 8.86059,0 16.01721,2.55024 16.01721,14.04128 v 41.62378 c 0,11.491053 -7.15662,14.051283 -16.01721, 14.051283 H 528.36145 82.130808 48.464716 c -22.802065,0 -26.654044,-0.91008 -26.654044,-13.231213 v -5.02046 c 0,-12.31111 3.862306,-13.2312 26.654044,-13.2312 h  10.161789 z m 474.475042,4.04037 c 0,9.03082 -2.92255,13.32121 -16.11015,13.32121 h -13.18761 c -12.05164,0 -15.92427,-4.93045 -15.92427,-13.41122 v -8.8508 c 0, -11.13101 6.69191,-12.95118 17.6179,-12.95118 h 10.92599 c 11.11188,0 16.66781,2.01019 16.66781,12.77116 v 9.12083 z"/> </svg> </span> </html>
)=====";
#endif