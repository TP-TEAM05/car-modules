import socket
import rclpy
from rclpy.node import Node

from std_msgs.msg import String


class MinimalPublisher(Node):

    def __init__(self):
        super().__init__('minimal_publisher')
        self.publisher_ = self.create_publisher(String, 'controls', 1) # String is the message type, topic is the topic name, 1 is the queue size - how many messages to store in the queue if messages are sent faster than they are published
        # queue size is important because if the queue is full, the oldest message will be dropped, we want 1 because we only care about the most recent message
        timer_period = 0.001  # seconds
        self.timer = self.create_timer(timer_period, self.timer_callback)

    def timer_callback(self):
        msg = String()
        data, addr = sock.recvfrom(1024)
        msg.data = data.decode() # distance in cm
        self.publisher_.publish(msg)
        self.get_logger().info('Publishing: "%s"' % msg.data)

# Define the IP address and port to listen on
HOST = '192.168.1.122'
PORT = 12345

# Create a socket for listening
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((HOST, PORT))

def main(args=None):
    try:
        rclpy.init()
        minimal_publisher = MinimalPublisher()
        rclpy.spin(minimal_publisher)
    except KeyboardInterrupt:
        # Cleanup when the program is terminated
        sock.close()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
