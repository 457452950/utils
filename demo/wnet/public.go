package main

import (
	"bytes"
	"fmt"
	"net"
	"syscall"
	"time"
)

func collect(input <-chan int) {
	myTimer := time.NewTicker(time.Second * 1) // 启动定时器
	c := 0
	n := 0

	fmt.Print("collect")

	for {
		select {
		case <-myTimer.C:
			fmt.Println(n/1024, "k bytes ", c, "times ")
			n = 0
			c = 0
		default:
			// fmt.Print("input...")
			i := <-input
			n += i
			c++
		}
	}

	myTimer.Stop()
}

func testTcp() {
	ch := make(chan int, 1000)
	go collect(ch)

	fmt.Println("test Tcp")

	f := func() {
		socket, err := net.Dial("tcp6", "[0:0:0:0:0:0:0:1]:4000")
		if err != nil {
			fmt.Println("连接tcp服务器失败，err: ", err)
			syscall.Exit(-1)
		}
		defer func(socket net.Conn) {
			err := socket.Close()
			if err != nil {

			}
		}(socket)

		fmt.Print("new connect\n")

		// fmt.Println("start f")
		for {
			sendData := []byte("asdasdasdasdasdasdasdasdasdasasfasfsaafabfubfusbfuwkebfusdbfsdfbjsbfsjdffuskbfksbfkbskjsbjkdasdasdsfasfvaababfeafbaebfabf")
			_, err := socket.Write(sendData) // 发送数据
			if err != nil {
				fmt.Println("发送数据失败，err: ", err)
				syscall.Exit(-1)
			} else {
				// fmt.Println("send len:", n)
			}
			data := make([]byte, 0)

			for {
				tmp := make([]byte, 1024)
				n, err := socket.Read(tmp) // 接收数据
				if err != nil {
					fmt.Println("接收数据失败, err: ", err)
					syscall.Exit(-1)
					return
				} else {
					// fmt.Println("recv len ", n)
				}
				//fmt.Println("recv data[", string(tmp), "]")
				data = append(data, tmp[:n]...)
				//fmt.Println("data len ", len(data), "send len", len(sendData))
				if len(data) == len(sendData) {
					//fmt.Println("recv over ")
					break
				}
			}
			if bytes.Equal(sendData, data) {
				//fmt.Println("equal")
				ch <- len(sendData)
			} else {
				fmt.Println("not equal")
				syscall.Exit(-1)
			}

		}
	}

	for i := 0; i < 15; i++ {
		go f()
	}

	select {}
}

func test_tcp2() {
	ch_c := make(chan int, 1000)
	go collect(ch_c)
	ch := make(chan int, 200)

	f := func() {
		socket, err := net.Dial("tcp6", "[0:0:0:0:0:0:0:1]:4000")
		if err != nil {
			fmt.Println("连接tcp服务器失败，err: ", err)
			syscall.Exit(-1)
		}
		defer func(socket net.Conn) {
			err := socket.Close()
			// fmt.Println("close !")
			if err != nil {

			}
		}(socket)

		//fmt.Print("connect\n")

		// fmt.Println("start f")
		sendData := []byte("asdasdasdasdasdasdasdasdasdasasfasfsaafabfubfusbfuwkebfusdbfsdfbjsbfsjdffuskbfksbfkbskjsbjkdasdasdsfasfvaababfeafbaebfabf")
		_, err = socket.Write(sendData) // 发送数据
		if err != nil {
			fmt.Println("发送数据失败，err: ", err)
			syscall.Exit(-1)
		} else {
			//fmt.Println("send len:", n)
		}
		data := make([]byte, 0)

		for {
			tmp := make([]byte, 1024)
			n, err := socket.Read(tmp) // 接收数据
			if err != nil {
				fmt.Println("接收数据失败, err: ", err)
				syscall.Exit(-1)
				return
			} else {
				//fmt.Println("recv len ", n)
			}
			//fmt.Println("recv data[", string(tmp), "]")
			data = append(data, tmp[:n]...)
			//fmt.Println("data len ", len(data), "send len", len(sendData))
			if len(data) == len(sendData) {
				//fmt.Println("recv over ")
				break
			}
		}
		if bytes.Equal(sendData, data) {
			// fmt.Println("equal")
			ch_c <- len(sendData)
		} else {
			fmt.Println("not equal")
			syscall.Exit(-1)
		}
		<-ch
	}

	fmt.Println("start test")

	//go func() {
	//	for {
	//		select {
	//		case ch <- 1:
	//			fmt.Println("add")
	//		}
	//	}
	//}()

	for {
		select {
		case ch <- 1:
			// fmt.Println("new go()")
			go f()
		}
	}
}

// UDP 客户端
func main() {

	testTcp()
	// test_tcp2()

}
