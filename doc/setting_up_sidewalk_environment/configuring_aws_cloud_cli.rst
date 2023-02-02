
.. _configuring_aws_cloud:

Configuring AWS Cloud
#####################

The following sections describe how to create the AWS IoT rule and IAM role for your Sidewalk destination to send messages to specified topic.

.. _configuring_aws_cloud_in_cli:

Configuring AWS Cloud with AWS CLI
**********************************

#. Create an IAM role that grants the AWS IoT Core for Sidewalk permission to send data to the IoT rule.

   a. Create a :file:`trust-policy.json` file with policy in JSON format:

      .. code-block:: JSON

         {
           "Version": "2012-10-17",
           "Statement": [
             {
               "Effect": "Allow",
               "Principal": {
                 "Service": "iotwireless.amazonaws.com"
               },
               "Action": "sts:AssumeRole"
             }
           ]
         }
      
   #. Use ``create-role`` CLI command to create role: 

      .. code-block:: console

         $ aws iam create-role --role-name SidewalkRole --assume-role-policy-document file://trust-policy.json

      The command will return your role details with an ``"Arn"``:

      .. code-block:: JSON

         {
             "Role": {
                 "Path": "/",
                 "RoleName": "SidewalkRole",
                 "RoleId": "ABCDEF1ABCDEF2ABCDEF3",
                 "Arn": "arn:aws:iam::123456789012:role/SidewalkRole",
                 "AssumeRolePolicyDocument": {
                     "Version": "2012-10-17",
                     "Statement": [
                         {
                             "Effect": "Allow",
                             "Principal": {
                                 "Service": "iotwireless.amazonaws.com"
                             },
                             "Action": "sts:AssumeRole"
                         }
                     ]
                 }
             }
         }

#. Create a destination that associates your Sidewalk device with this IoT rule.

   .. code-block:: console

      $ aws iotwireless create-destination --name SidewalkDestination --expression-type RuleName --expression <IOT_RULE_NAME> --role-arn <SIDEWALK_ROLE_ARN>

   See an example:

   .. code-block:: console

      $ aws iotwireless create-destination --name SidewalkDestination --expression-type MqttTopic --expression "project/sensor/observed" --role-arn arn:aws:iam::123456789012:role/SidewalkRole

   The command will return your destination Name and Arn:

   .. code-block:: JSON

      {
          "Arn": "arn:aws:iotwireless:us-east-1:123456789012:Destination/SidewalkDestination",
          "Name": "SidewalkDestination"
      }

Expected output
***************

At the end of this step you need to have the following:

* Completed association between AWS and your Amazon ID.

* IAM permission policies and roles for your destinations and rules.

   .. note::
       This is to ensure that AWS can publish data to your destination and rules.

* Destination and rule which determine how the data incoming from your Sidewalk Endpoints are handled.

* Rule actions to republish uplink data to an MQTT topic.


.. _Sidewalk developer Service (SDS) console: http://developer.amazon.com/acs-devices/console/Sidewalk
.. _AWS Management Console: https://us-east-1.console.aws.amazon.com/iot/home?region=us-east-1#/home
.. _Add your Sidewalk account credentials: https://docs.aws.amazon.com/iot/latest/developerguide/iot-sidewalk-add-credentials.html
.. _Functions: https://console.aws.amazon.com/lambda/home?region=us-east-1#/functions
.. _Rules: https://console.aws.amazon.com/iot/home?region=us-east-1#/rulehub
.. _Create an IAM role for your destinations: https://docs.aws.amazon.com/iot/latest/developerguide/connect-iot-lorawan-create-destinations.html#connect-iot-lorawan-create-destinations-roles
.. _Using IAM roles: https://docs.aws.amazon.com/IAM/latest/UserGuide/id_roles_use.html
.. _Create rules to process Sidewalk device messages: https://docs.aws.amazon.com/iot/latest/developerguide/iot-sidewalk-create-rules.html
.. _Destinations: https://console.aws.amazon.com/iot/home#/wireless/destinations
.. _Create rules to process LoRaWAN device messages: https://docs.aws.amazon.com/iot/latest/developerguide/connect-iot-lorawan-destination-rules.html
.. _Add a destination for your Sidewalk device: https://docs.aws.amazon.com/iot/latest/developerguide/iot-sidewalk-add-destination.html

