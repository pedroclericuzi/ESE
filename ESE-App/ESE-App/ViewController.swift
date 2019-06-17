//
//  ViewController.swift
//  ESE-App
//
//  Created by Pedro Clericuzi on 17/06/19.
//  Copyright © 2019 Pedro Clericuzi. All rights reserved.
//

import UIKit
import GoogleMaps
import Firebase
import FirebaseDatabase

class ViewController: UIViewController {
    var ref:DatabaseReference!
    override func viewDidLoad() {
        super.viewDidLoad()
        
        ref = Database.database().reference()
        
        self.ref.child("RPI").observe(DataEventType.value, with: { (snapshot) in
            let postDict = snapshot.value as? NSDictionary
            //self.meta.text = ("\(postDict!["meta"] ?? "")")
            print(("\(postDict!["Latitude"] ?? "")"))
            //let lati = Double(postDict!["Latitude"] ?? "")
            let latDisp = Double("\(postDict!["Latitude"] ?? "-8.0556681")")
            let longDisp = Double("\(postDict!["Longitude"] ?? "-34.951578")")
            
            // Do any additional setup after loading the view.
            let camera = GMSCameraPosition.camera(withLatitude: latDisp!, longitude: longDisp!, zoom: 15.0)
            let mapView = GMSMapView.map(withFrame: CGRect.zero, camera: camera)
            self.view = mapView
            
            // Creates a marker in the center of the map.
            let marker = GMSMarker()
            marker.position = CLLocationCoordinate2D(latitude: latDisp!, longitude: longDisp!)
            marker.title = "Gateway"
            marker.map = mapView
            
            
            self.ref.child("Dispositivo").observe(DataEventType.value, with: { (snapshot) in
                let postDict = snapshot.value as? NSDictionary
                //let lati = Double(postDict!["Latitude"] ?? "")
                let latDisp = Double("\(postDict!["Latitude"] ?? "-8.0556681")")
                let longDisp = Double("\(postDict!["Longitude"] ?? "-34.951578")")
                
                // Creates a marker in the center of the map.
                let markerImage = UIImage(named: "pin")!.withRenderingMode(.alwaysTemplate)
                let marker = GMSMarker()
                marker.position = CLLocationCoordinate2D(latitude: latDisp!, longitude: longDisp!)
                marker.title = "Criança"
                marker.map = mapView
                marker.icon = markerImage
            })
        })
    }
}

