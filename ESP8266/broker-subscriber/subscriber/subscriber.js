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
		console.log("Localizacao atual: " + messagem);
		//dbfirebase.enviarLocalizacao(lat,long)
	}
})

/* O consumo de água, segundo: https://lojavirtual.compesa.com.br:8443/gsan/exibirConsultarEstruturaTarifariaPortalAction.do
Até 10.000 litros/mês 	41,30
10.001 a 20.000 litros 	4,74
20.001 a 30.000 litros 	5,63
30.001 a 50.000 litros 	7,75
50.001 a 90.000 litros 	9,18
90.001 a 999999.000 litros 	17,65 */
function readWater(msg){
	of([msg])
	.pipe(reduce((total,price) => parseFloat(total) + parseFloat(price), litros))
	.subscribe(dado => {
		console.log("dado " + litros);
		var msg = parseFloat(dado);
		litros = msg;
		console.log("litros " + litros);
		if(cmpData (diaLeituraAgua, mesLeituraAgua)===true){
			console.log("data igual");
			mesLeituraAgua = (mesLeituraAgua%12) + 1;
			dbfirebase.definicoes(diaLeituraAgua, mesLeituraAgua, metaAgua,'agua');
			dbfirebase.statusAgua(0, 0, 0);
			litros = 0;
		} else {
			const valores = of(41.3, 4.74, 5.63, 7.75, 9.18, 17.65);
			var takeIndex; 
			if(litros<=10000){
				takeIndex = valores.pipe(take(1));
			} else if(litros>10000 && litros<=20000){
				takeIndex = valores.pipe(take(2));
			} else if(litros>20000 && litros<=30000){
				takeIndex = valores.pipe(take(3));
			} else if(litros>30000 && litros<=50000){
				takeIndex = valores.pipe(take(4));
			} else if(litros>50000 && litros<=90000){
				takeIndex = valores.pipe(take(5));
			} else if (litros>90000){
				takeIndex = valores.pipe(take(6));
			}

			const reduceVal = takeIndex.pipe(reduce((acc, val) => acc + val));
			reduceVal.subscribe(val => dbfirebase.statusAgua(litros,0,val));
		}
	});
}

//Valor da tarifa da distribuidora Celpe em junho de 2018 corresponde a 0.75
function readEnergy(value){
	of([value])
	.pipe(map(num => ((num / 1000) * 0.00027778)*0.75))
	.pipe(reduce((total,price) => total + price, valorReal))
	.subscribe(dado => {
			valorReal = dado;
			if(cmpData (diaLeituraEnergia, mesLeituraEnergia) === true){
				//Se o mes for igual, mudo a data aqui e mando pro banco
				mesLeituraEnergia = (mesLeituraEnergia%12) + 1;
				dbfirebase.definicoes(diaLeituraEnergia, mesLeituraEnergia, metaEnergia,'energia');
				dbfirebase.statusEnergia(0, 0, 0);
				valorReal = 0;
				console.log("mes: "+mesLeituraEnergia);
			} else {
				var porcentagem = (valorReal * 100)/metaEnergia;
				if(porcentagem<80){
					dbfirebase.statusEnergia(porcentagem, 0, valorReal);
					console.log("valorReal < 80% = " + porcentagem);
				}
				if(porcentagem>=80 && porcentagem<100){ //evento disparado aqui
					dbfirebase.statusEnergia(porcentagem, 1, valorReal);
					console.log("valorReal entre 80% = " + porcentagem);
				}
				if(porcentagem>=100){ //evento disparado aqui
					dbfirebase.statusEnergia(porcentagem, 2, valorReal);
					console.log("valorReal igual ou acima de 100% = " + porcentagem);
				}
			}
		},
		error => console.log("Erro: "+error)
	);
}

function cmpData (diaLeitura, mesLeitura){
	var d = new Date();
	var idDtHoje = d.getDate()+""+(d.getMonth() + 1);
	var idDtLeitura = diaLeitura+""+mesLeitura;
	var cmpDatas = [idDtHoje, idDtLeitura];
	if(cmpDatas[0] === cmpDatas[1]){
		return true;
	} else{
		return false;
	}
}