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

.. _Sidewalk console: https://developer.amazon.com/acs-devices/console/sidewalk/products
.. _AWS Management Console: https://console.aws.amazon.com/console/home?region=us-east-1

