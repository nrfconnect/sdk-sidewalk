.. _generating_app_server_keys:

Generating certificate
######################

To generate application server certificates, use the tool provided in the SDK tools folder.

   #. Find the script by navigating to the Sidewalk tools folder of the SDK.

      .. code-block:: console

         $ cd ${ZEPHYR_BASE}/../sidewalk/tools/application_server_cert

   #. Run the application key generation script to generate your application server certificates.

      .. code-block:: console

         $ python3 generate_application_server.py --app_srv_serial <ApplicationServerSerialNumber> \
         --apid <APID>

      .. note::

          * ApplicationServerSerialNumber is a 4-byte, base10 integer (0 to 4294967295) that you define.
          * APID consists of 4-digit alphanumeric value.
            For now you can assign any value.

      .. code-block:: console

         # Sample command:
         $ python3 generate_application_server.py --app_srv_serial 1234567890 --apid cafe
         $ ls app-server*

         #  Sample output:
         app-server-ed25519-csr.bin
         app-server-ed25519.private.bin
         app-server-p256r1-signature.bin
         app-server-ed25519-private.pem
         app-server-ed25519.public.bin
         app-server-p256r1-signature.der
         app-server-ed25519-private.txt
         app-server-p256r1-csr.bin
         app-server-p256r1.c
         app-server-ed25519-public.pem
         app-server-p256r1-private.pem
         app-server-p256r1.private.bin
         app-server-ed25519-public.txt
         app-server-p256r1-private.txt
         app-server-p256r1.public.bin
         app-server-ed25519-signature.bin
         app-server-p256r1-public.pem
         app-server-ed25519.c
         app-server-p256r1-public.txt

   .. note::
       Once the script is executed, several keys will be generated.
       Only two of them are required in the next steps.

       * :file:`app-server-ed25519-private.txt` - an input to configure your AWS cloud in :ref:`configuring_aws_cloud`.
       * :file:`app-server-ed25519.public.bin` - an input to :file:`provision.py` script as the application server public key.
