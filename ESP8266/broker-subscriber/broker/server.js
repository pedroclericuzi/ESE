var mosca = require('mosca');
var server = new mosca.Server({
	//host:'192.168.0.15',
	host:'192.168.10.107',
	port:3000
});

server.on('clientConnected', function(client){
	console.log('Cliente conectado: %s', client.id);
});

server.on('published', function(packet, client){
	console.log(packet);
});

server.on('subscribe', function(topic, client){
	console.log('received message %s %s', topic, client.id);
});

server.on('clientDisconnected', function(client){
	console.log('Cliente %s esta desconectado', client.id);
});

server.on('ready', function(){
	console.log('Servidor mosca esta executando');
});

server.on('error', function(err){
	console.log('erro: %s', err);
});