
.. _configuring_aws_cloud:

Configuring AWS Cloud
#####################

The section outlines how to create a Sidewalk wireless connectivity profile in AWS cloud account and how to create a new AWS IoT destination rule for your profile and Sidewalk Endpoints.
Rules can be triggered with specific actions (for example, Lambda function), to handle incoming messages and events received from your Sidewalk Endpoints.

The data flow between your Sidewalk Endpoint and cloud service is as follows:

   .. figure:: /images/sidewalk_cloud_image.jpg

* Device-to-Cloud flow:

   * Sidewalk Endpoint → Sidewalk via Sidewalk Device SDK → AWS IoT Core → Store/Process (DynamoDB/Lambda).

* Cloud-to-device flow:

   * AWS IoT SendDataToWirelessDevice API → Sidewalk → Sidewalk Endpoint via Sidewalk SDK.

.. _configuring_aws_cloud_req:

Requirements
************

* Ensure your AWS user account has the required IAM permissions to proceed with certain administrative operations.

.. _configuring_aws_cloud_association:

Associating Sidewalk product with AWS IoT Core
**********************************************

You can connect Sidewalk devices to AWS IoT by using the AWS Management Console or the AWS IoT Wireless API.

#. To onboard your device, create a wireless connectivity profile for your Sidewalk device.
#. Follow instructions below to add destination and AWS IoT rule for the profile and Sidewalk endpoints.

.. _configuring_aws_cloud_credentials:

Adding your sidewalk credentials via console
********************************************

You can add your credentials by using the AWS Management Console or the AWS IoT Wireless API.
To add your Sidewalk account credentials via console, follow the steps below.

#. Go to `Sidewalk Developer Service (SDS) console`_.

#. Copy your Amazon ID.

   .. figure:: /images/SidewalkProductConfiguration-AmazonID.jpg

#. Navigate to :guilabel:`Profiles` in the AWS IoT console and select :guilabel:`Sidewalk` tab.

   .. note::
      Ensure you are using :guilabel:`us-east-1` region.
      If you are using a different region, this tab will not appear.

   .. figure:: /images/AWSIoT-selectsidewalk.jpg

#. Click :guilabel:`Add credential`.

   .. figure:: /images/AWSIoT-AddCredential.jpg

#. Enter your Sidewalk Amazon ID.

   .. figure:: /images/AWSIoT-EnterAmazonID.jpg

#. Upload AppServerPrivateKey, which is the server key provided by your vendor.

   AppServerPrivateKey is the ED25519 private key (the :file:`app-server-ed25519-private.txt` file), which is a 64-digit hexadecimal value that you generate by using the Sidewalk certificate generation tool when designing your Sidewalk product.

   .. figure:: /images/AWSIoT-UploadAppServerPrivateKey.jpg

#. To add your sidewalk credentials, click :guilabel:`Add credential`.

   .. figure:: /images/AWSIoT-AddCredential2.jpg

   A new item will appear on the :guilabel:`Sidewalk account credentials` list.

   .. figure:: /images/AWSIoT-SidewalkAccountCredentialList.jpg

   .. note::
      To add credentials via API refer to `Add your Sidewalk account credentials`_.

.. _configuring_aws_Lambda_function:

Creating function using AWS Lambda
**********************************

#. Open `Functions`_ page of the Lambda console.

#. Click :guilabel:`Create function`.

   .. figure:: /images/LambdaFunctions-CreateFunction.jpg

#. Choose :guilabel:`Author from scratch`.

   .. figure:: /images/LambdaFunctions-AuthorFromScratch.jpg

#. In :guilabel:`Function name` enter a name (for example, ``Sidewalk_Handler_Demo``).

#. In :guilabel:`Runtime` choose :guilabel:`Python 3.8`.
#. Click :guilabel:`Create function`.

   .. figure:: /images/LambdaFunctions-CreateFunction2.jpg

#. Delete any code inside the function body and add a print statement for your Lambda function.
   You can also base64 decode the :file:`PayloadData` to receive the application data that your device sends to AWS IoT.
   See an example of Lambda function:

   .. code-block:: console

      python
      import json
      import base64

      def lambda_handler(event, context):

         message = json.dumps(event)
         print (message)

         payload_data = base64.b64decode(event["PayloadData"])
         print(payload_data)
         print(int(payload_data,16))

   .. figure:: /images/LambdaFunctions-DeployNewCode.jpg

#. To deploy your function code, choose :guilabel:`deploy`.

#. Go back to `Functions`_ and check if the function appears on the list.

   .. figure:: /images/LambdaFunctions-FunctionsList.jpg

.. _configuring_destination_rule:

Creating Sidewalk destination rule
**********************************

AWS IoT rules can receive the messages from Sidewalk devices and route them to other services.
AWS IoT Core for LoRaWAN destinations associate a Sidewalk device with the rule that processes the device's message data to send to other services.

#. Navigate to `Rules`_ Hub of the AWS IoT console.

#. Click :guilabel:`Create a rule` to create a new rule for the destination.

   .. figure:: /images/AWSIoT-Rule-CreateNew.jpg

   .. note::
      The AWS IoT rule is configured to process the device's data.
      Your destination needs a rule to process the messages it receives.

#. In :guilabel:`Name` field enter ``SidewalkRuleDemo``.

#. In optional :guilabel:`Description` field specify the rule, for example, ``Sidewalk rule for lamba action to republish a topic``.

   .. figure:: /images/AWSIoT-Rule-NameAndDescription.jpg

#. Change the default query statement to :guilabel:`SELECT *` so that any actions associated with the rule can be performed.

#. Keep the :guilabel:`2016-03-23` SQL version.

   .. figure:: /images/AWSIoT-Rule-QueryStatement.jpg

#. Under :guilabel:`Set one or more actions` choose :guilabel:`Add action`.

   .. figure:: /images/AWSIoT-Rule-AddAction.jpg

#. For the rule action choose :guilabel:`Send a message to a Lambda function` and click :guilabel:`Configure action`.

   .. figure:: /images/AWSIoT-Rule-SelectAnAction-LambdaFunction.jpg

#. Either choose an existing Lambda function by clicking :guilabel:`Select`, or create a new one.

   .. figure:: /images/AWSIoT-Rule-SelectAnAction-SelectLambdaFunction.jpg

   The added action will appear in :guilabel:`Set one or more actions`.

#. Click :guilabel:`Add action` button to add the next action.

   .. figure:: /images/AWSIoT-Rule-AddNextAction.jpg

#. Choose :guilabel:`Republish a message to an AWS IoT topic` and click :guilabel:`Configure action`.

   .. figure:: /images/AWSIoT-Rule-SelectAnAction-Republish.jpg

#. In the :guilabel:`Topic` field enter ``project/sensor/observed`` and ensure the :guilabel:`Quality of Service` is set to ``0 - The message is delivered zero or more times``.

   .. figure:: /images/AWSIoT-Rule-SelectAnAction-Republish-Configure.jpg

#. Click :guilabel:`Create role` button.

   .. figure:: /images/AWSIoT-Rule-SelectAnAction-Republish-CreateRole.jpg

#. Enter role name and click :guilabel:`Create role`.

   .. figure:: /images/AWSIoT-Rule-SelectAnAction-Republish-CreateRolePopup.jpg

   .. note::

    The IAM role gives the device's data permission to access the rule.
    To create the IAM role, follow the steps described in `Create an IAM role for your destinations`_.
    For more information on IAM roles, refer to `Using IAM roles`_.

#. Click :guilabel:`Add action`.

   .. figure:: /images/AWSIoT-Rule-SelectAnAction-Republish-AddAction.jpg

   The added action will appear in :guilabel:`Set one or more actions`.

   .. figure:: /images/AWSIoT-Rule-SetActionCategory.jpg

#. Go to the bottom of :guilabel:`Create a rule` page and click :guilabel:`Create rule`.
   New rule will appear on the :guilabel:`Rules` list.

   .. figure:: /images/AWSIoT-Rule-RuleCreated.jpg

   .. note::
      For more details on creating rules, refer to `Create rules to process Sidewalk device messages`_.

Adding a destination via console
********************************

You can add your Sidewalk destination from the `Destinations`_ page of the AWS IoT console.

#. Click :guilabel:`Add destination`.

   .. figure:: /images/AWSIoT-Destination-DestList-AddDest.jpg

#. Enter a destination name and an optional description for your destination.

      * In the :guilabel:`Destination name` field, enter ``SidewalkDestinationDemo``.
      * You can optionally enter a description, for example, ``This is a destination for Sidewalk devices``.
      * Enter a rule name ``SidewalkRuleDemo`` created in previous steps.

   .. note::
      The AWS IoT rule is configured to process the device's data.
      Your destination needs a rule to process the messages it receives.

   .. figure:: /images/AWSIoT-Destination-NewDestConfiguration.jpg

   .. note::
      For more information on AWS IoT rules for destinations, see `Create rules to process LoRaWAN device messages`_.

#. On the bottom of :guilabel:`Add destination` page click :guilabel:`Add destination` .
   New destination will appear on the destinations list.

   .. figure:: /images/AWSIoT-Destination-DestList.jpg

   .. note::
      To add destination via API, refer to `Add a destination for your Sidewalk device`_.

Expected output
***************

At the end of this step you need to have the following:

* Completed association between AWS and your Amazon ID.

   .. note::
       It will enable you to provision and generate device keys within the Sidewalk Developer Services console for prototyping. These wireless devices will show up in AWS IoT Core. The device keys, along with the application server public key are used to create the manufacturing page and to provision the device for Sidewalk registration.

* IAM permission policies and roles for your destinations and rules.

   .. note::
       This is to ensure that AWS can publish data to your destination and rules.

* Destinations and rules which determine how the data incoming from your Sidewalk Endpoints are handled.
* Rule actions to republish uplink data to an MQTT topic and a Lambda function.


.. _Sidewalk developer Service (SDS) console: http://developer.amazon.com/acs-devices/console/Sidewalk
.. _Add your Sidewalk account credentials: https://docs.aws.amazon.com/iot/latest/developerguide/iot-sidewalk-add-credentials.html
.. _Functions: https://console.aws.amazon.com/lambda/home?region=us-east-1#/functions
.. _Rules: https://console.aws.amazon.com/iot/home?region=us-east-1#/rulehub
.. _Create an IAM role for your destinations: https://docs.aws.amazon.com/iot/latest/developerguide/connect-iot-lorawan-create-destinations.html#connect-iot-lorawan-create-destinations-roles
.. _Using IAM roles: https://docs.aws.amazon.com/IAM/latest/UserGuide/id_roles_use.html
.. _Create rules to process Sidewalk device messages: https://docs.aws.amazon.com/iot/latest/developerguide/iot-sidewalk-create-rules.html
.. _Destinations: https://console.aws.amazon.com/iot/home#/wireless/destinations
.. _Create rules to process LoRaWAN device messages: https://docs.aws.amazon.com/iot/latest/developerguide/connect-iot-lorawan-destination-rules.html
.. _Add a destination for your Sidewalk device: https://docs.aws.amazon.com/iot/latest/developerguide/iot-sidewalk-add-destination.html

