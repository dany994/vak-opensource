package main

import "stream"

var x int = 12345

func Start(console stream.Stream_t) {
	console.Put_String ("Hello, World!\n")

	console.Put_String ("Global variable x = ")
	console.Put_Integer (x, 0, 10)
	console.Put_String ("\n")
	x += x;
}
