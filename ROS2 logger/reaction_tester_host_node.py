# microROS host node for the reaction tester project
import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32
import datetime

class ReactionTesterHostNode(Node):

    def __init__(self):
        super().__init__('reaction_tester_host_node')
        self.best_time = None
        
        # Defining the subscriber to the reaction times sent by Pico
        self.subscriber = self.create_subscription(
            Int32,
            'reaction_time',
            self.reaction_time_received_callback,
            10
        )

        #Defining the publisher that will send the best time currently recorded
        self.publisher = self.create_publisher(
            Int32,
            'best_reaction_time',
            10
        )

        self.get_logger().info('Reaction Tester Host Node has been started.')

    # Handle receiving reaction time messages
    def reaction_time_received_callback(self, msg):
        current_time = msg.data
        message_timestamp = datetime.datetime.now()
        self.get_logger().info(f'{message_timestamp} - reaction time received: {current_time} ms')

        # Check if the current time is better than the best time recorded (or is the first time recorded)
        if self.best_time is None or current_time < self.best_time:
            self.best_time = current_time
            self.get_logger().info(f'NEW RECORD at {message_timestamp}: {self.best_time} ms')
            best_time_msg = Int32()
            best_time_msg.data = self.best_time
            self.publisher.publish(best_time_msg)

# Main function
if __name__ == '__main__':
    rclpy.init()
    node = ReactionTesterHostNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
    print("Reaction Tester Host Node has been shut down.")