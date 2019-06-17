const firebase = require('firebase-admin');
const servicosConta = require("./serviceAccountKey.json");
// Initialize Firebase
firebase.initializeApp({
  credential: firebase.credential.cert(servicosConta), 
  databaseURL: "https://eseaplicativo-1558653308688.firebaseio.com"
});

exports.enviarLocalizacao = function(lat, long){
  firebase.database().ref('Dispositivo/').set({
    Latitude : lat,
    Longitude : long
  });
}