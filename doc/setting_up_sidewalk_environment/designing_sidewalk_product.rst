.. _designing_sidewalk_product:

Designing your Sidewalk product
###############################

To design your Sidewalk product you need to use Sidewalk console to define and create a new configuration for a given device.
Then, you need to link developer and AWS cloud accounts.

At the end of this section you need to have the following:

* Software bundle for your hardware development kit.
* Amazon ID (generated for your Sidewalk product).
  The Amazon ID is used to complete the association of the product from within your AWS account.

* 2 application server certificates:

   * :file:`App-server-ed25519.public.bin` key that is used in this section to create the manufacturing.
   * :file:`App-server-ed25519-private.txt` key that is used along with the Amazon ID to complete the association of your Sidewalk product in your AWS account.

#. Go to `Sidewalk console`_.
#. On the left hand side panel navigate to :guilabel:`Design`.
#. Define and create a new Sidewalk product by entering the following information about the product:

   * Name
   * Category
   * Brief description
   * Target launch date

#. Click :guilabel:`Configure product`.

   .. figure:: /images/DesignSidewalkProduct.jpg

#. Expand :guilabel:`Hardware Development Kit section`, and select your hardware development.
#. Save the new Sidewalk product configuration by clicking :guilabel:`Save Configuration`.

   .. figure:: /images/DesignSidewalkProduct-HDKselection.jpg

   .. note::
       Your Amazon ID is automatically generated for the Sidewalk product.
       Sidewalk Developer Services may take a few minutes to create and to save your new Sidewalk product configuration.

   .. figure:: /images/SidewalkProductConfiguration.jpg

#. Make note of your Amazon ID.

   .. figure:: /images/AmazonID.jpg

#. Go to :guilabel:`Sidewalk Product Configuration` page, and click :guilabel:`Configure Your Cloud`.

   .. figure:: /images/SidewalkProductConfiguration-ConfigureYourCloud.jpg

#. Go to `AWS Management Console`_, and copy your Amazon ID.

   .. figure:: /images/AWSAccountID.jpg

#. Enter your AWS account ID.

   .. figure:: /images/EnterAWSAccountID.jpg

#. Click :guilabel:`Link your Cloud Account`.

   .. figure:: /images/LinkCloudAccount.jpg

#. Sign in with your AWS account to set up and associate your AWS cloud account.

.. _configuring_aws_cloud_credentials:

Adding Sidewalk credentials from console
****************************************

You can add your credentials by using the AWS Management Console or the AWS IoT Wireless API.
To add your Sidewalk account credentials from console, follow the steps below.

#. Go to `Sidewalk console`_.

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

#. To add your Sidewalk credentials, click :guilabel:`Add credential`.

   .. figure:: /images/AWSIoT-AddCredential2.jpg

   A new item will appear on the :guilabel:`Sidewalk account credentials` list.

   .. figure:: /images/AWSIoT-SidewalkAccountCredentialList.jpg

   .. note::
      To add credentials through API refer to `AWS Management Console`_.

.. _endpoints_config_provisioning:

Provisioning Sidewalk Endpoints
*******************************

#. Go to a list of products in :guilabel:`My Products` section in the `Sidewalk console`_ and select previously created product.

   .. figure:: /images/Step5-ProductList.jpg

#. Click :guilabel:`Configure Your Cloud`.

   .. figure:: /images/Step5-ConfigureCloud.jpg

#. Click :guilabel:`Provision Device`.

   .. figure:: /images/Step5-ProvisionButton.jpg

#. Choose :guilabel:`Generate New Certificate`.
   This will provision new Sidewalk Endpoints and generate their public and private certificates.

   .. figure:: /images/Step5-GenNewCertButton.jpg

#. Note down the Sidewalk ID of this Sidewalk Endpoint.

   .. note::
    Sidewalk ID needs to be used for flashing the manufacturing page (contains the certificates) to the hardware.
    It is also required in order to send data to your Sidewalk Endpoint.

   .. figure:: /images/Step5-SidewalkID.jpg

#. Download the device key bundle by clicking :guilabel:`Download Certificate`.
   The key bundle will be flashed onto the Sidewalk Endpoint.

   .. figure:: /images/Step5-DownloadCerts.jpg

.. _Sidewalk console: https://developer.amazon.com/acs-devices/console/sidewalk/products
.. _Manufacturing: https://developer.amazon.com/acs-devices/console/sidewalk/docs/group__manufacturing.html
.. _AWS Management Console: https://console.aws.amazon.com/console/home?region=us-east-1

