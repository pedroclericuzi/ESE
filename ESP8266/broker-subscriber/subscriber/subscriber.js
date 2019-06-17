const { Observable, Subject, ReplaySubject, from, interval, of, timer } = require('rxjs');
const { map, filter, reduce, take } = require('rxjs/operators');
const mqtt = require('mqtt');
const dbfirebase = require('./dbFirebase.js');
const client = mqtt.connect('mqtt://192.168.10.107:3000');

client.on('connect', () => {
    console.log('connected');
    client.subscribe("latlong");
})

client.on('message', (topico, messagem) => {
	if(topico == "latlong"){
		var coord = messagem + "";
		console.log("Localizacao atual: " + coord);
		coord = coord.split("/");
		dbfirebase.enviarLocalizacao(coord[0],coord[1])
	}
})