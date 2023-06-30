
# cpp-transport-catalogue
Финальный проект: транспортный справочник

Построение автобусных маршрутов и поиск оптимального пути между остановками  
  
## Пример  
  
### Ввод:
```
{  
  "base_requests": [  
	  {  
		  "is_roundtrip": true,  
		  "name": "297",  
		  "stops": [  
			  "Biryulyovo Zapadnoye",  
			  "Biryulyovo Tovarnaya",  
			  "Universam",  
			  "Biryulyovo Zapadnoye"  
		  ],  
		  "type": "Bus"  
	  },  
	  {  
		  "is_roundtrip": false,  
		  "name": "635",  
		  "stops": [  
			  "Biryulyovo Tovarnaya",  
			  "Universam",  
			  "Prazhskaya"  
		  ],  
		  "type": "Bus"  
	  },  
	  {  
		  "latitude": 55.574371,  
		  "longitude": 37.6517,  
		  "name": "Biryulyovo Zapadnoye",  
		  "road_distances": {  
			  "Biryulyovo Tovarnaya": 2600  
		  },  
		  "type": "Stop"  
	  },  
	  {  
		  "latitude": 55.587655,  
		  "longitude": 37.645687,  
		  "name": "Universam",  
		  "road_distances": {  
			  "Biryulyovo Tovarnaya": 1380,  
			  "Biryulyovo Zapadnoye": 2500,  
			  "Prazhskaya": 4650  
		  },  
		  "type": "Stop"  
	  },  
	  {  
		  "latitude": 55.592028,  
		  "longitude": 37.653656,  
		  "name": "Biryulyovo Tovarnaya",  
		  "road_distances": {  
			  "Universam": 890  
		  },  
		  "type": "Stop"  
	  },  
	  {  
		  "latitude": 55.611717,  
		  "longitude": 37.603938,  
		  "name": "Prazhskaya",  
		  "road_distances": {},  
		  "type": "Stop"  
	  }  
  ],  
  "render_settings": {  
	  "bus_label_font_size": 20,  
	  "bus_label_offset": [  
		  7,  
		  15  
	  ],  
	  "color_palette": [  
		  "green",  
		  [  
			  255,  
			  160,  
			  0  
		  ],  
		  "red"  
	  ],  
	  "height": 200,  
	  "line_width": 14,  
	  "padding": 30,  
	  "stop_label_font_size": 20,  
	  "stop_label_offset": [  
		  7,  
		  -3  
	  ],  
	  "stop_radius": 5,  
	  "underlayer_color": [  
		  255,  
		  255,  
		  255,  
		  0.85  
	  ],  
	  "underlayer_width": 3,  
	  "width": 200  
  },  
  "routing_settings": {  
	  "bus_velocity": 40,  
	  "bus_wait_time": 6  
  },  
  "stat_requests": [  
	  {  
		  "id": 1,  
		  "name": "297",  
		  "type": "Bus"  
	  },  
	  {  
		  "id": 2,  
		  "name": "635",  
		  "type": "Bus"  
	  },  
	  {  
		  "id": 3,  
		  "name": "Universam",  
		  "type": "Stop"  
	  },  
	  {  
		  "from": "Biryulyovo Zapadnoye",  
		  "id": 4,  
		  "to": "Universam",  
		  "type": "Route"  
	  },  
	  {  
		  "from": "Biryulyovo Zapadnoye",  
		  "id": 5,  
		  "to": "Prazhskaya",  
		  "type": "Route"  
	  }  
  ]  
}
```  
### Вывод:  
```
[  
  {  
	  "curvature": 1.42963,  
	  "request_id": 1,  
	  "route_length": 5990,  
	  "stop_count": 4,  
	  "unique_stop_count": 3  
  },  
  {  
	  "curvature": 1.30156,  
	  "request_id": 2,  
	  "route_length": 11570,  
	  "stop_count": 5,  
	  "unique_stop_count": 3  
  },  
  {  
	  "buses": [  
		  "297",  
		  "635"  
	  ],  
	  "request_id": 3  
  },  
  {  
	  "items": [  
		  {  
			  "stop_name": "Biryulyovo Zapadnoye",  
			  "time": 6,  
			  "type": "Wait"  
		  },  
		  {  
			  "bus": "297",  
			  "span_count": 2,  
			  "time": 5.235,  
			  "type": "Bus"  
		  }  
	  ],  
	  "request_id": 4,  
	  "total_time": 11.235  
  },  
  {  
	  "items": [  
		  {  
			  "stop_name": "Biryulyovo Zapadnoye",  
			  "time": 6,  
			  "type": "Wait"  
		  },  
		  {  
			  "bus": "297",  
			  "span_count": 2,  
			  "time": 5.235,  
			  "type": "Bus"  
		  },  
		  {  
			  "stop_name": "Universam",  
			  "time": 6,  
			  "type": "Wait"  
		  },  
		  {  
			  "bus": "635",  
			  "span_count": 1,  
			  "time": 6.975,  
			  "type": "Bus"  
		  }  
	  ],  
	  "request_id": 5,  
	  "total_time": 24.21  
  }  
]  
```
<img width="237" alt="image" src="https://github.com/H1eb/cpp-transport-catalogue/assets/66701341/28a6b92e-c6a2-4359-bf42-f9c543756a45">
<img width="241" alt="image" src="https://github.com/H1eb/cpp-transport-catalogue/assets/66701341/b2959e7d-2461-418f-9303-85b60f4414b6">
<img width="253" alt="image" src="https://github.com/H1eb/cpp-transport-catalogue/assets/66701341/1f30ebcd-ee10-4fad-aa7a-c97c52f37639">
<img width="266" alt="image" src="https://github.com/H1eb/cpp-transport-catalogue/assets/66701341/fc93810e-1bfa-49ec-9a66-5cf057a106b1">

