import rclpy
from rclpy.node import Node
from std_msgs.msg import String

#ser = serial.Serial ("/dev/serial0", 115200)

class MyNode(Node):
    def __init__(self):
        super().__init__("minimal_publisher")
        self.distance_sub = self.create_subscription(String, "distance", self.distance_callback, 10)
        self.controls_sub = self.create_subscription(String, "controls", self.controls_callback, 10)
        self.controls_enabled = True  # Controls enabled flag

    def distance_callback(self, msg):
        distance = int(msg.data)
        if distance < 100:
            #self.controls_enabled = False
            self.get_logger().info("<0,0,0,0>".format(msg.data))
            #ser.write(b"<0,0,0,0>")
        else:
            #self.controls_enabled = True
            self.get_logger().info("Distance: {}".format(distance))

    def controls_callback(self, msg):
        if self.controls_enabled:
            self.get_logger().info("Controls: {}".format(msg.data))
            #ser.write(msg.data.encode())

def main(args=None):
    try:
        rclpy.init(args=args)
        node = MyNode()
        rclpy.spin(node)
        
    except KeyboardInterrupt:
        # Cleanup when the program is terminated
        rclpy.shutdown()

if __name__ == "__main__":
    main()
