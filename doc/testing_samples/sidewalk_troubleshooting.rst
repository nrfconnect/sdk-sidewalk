.. _sidewalk_troubleshooting:

Troubleshooting
###############

Refer to the information below for common errors and their solutions.

Failed to initialize Sidewalk error code -8
===========================================

Log:

   .. code-block:: console

      *** Booting Zephyr OS build v3.0.99-ncs1-4913-gf7b06162027d  ***
      [00:00:00.006,317] <inf> sid_template: Sidewalk example started!
      [00:00:00.020,690] <inf> sid_thread: Start Sidewalk link_mask:1
      [00:00:00.021,362] <err> sid_thread: failed to initialize sidewalk link_mask:1, err:-8
      [00:00:00.021,362] <inf> sid_thread: Deinitialize Sidewalk, link_mask:1
      [00:00:00.021,392] <err> sid_thread: Sidewalk stop error (code -11)
      [00:00:00.021,392] <err> sid_thread: Sidewalk deinit error (code -11)
      [00:00:00.021,423] <err> sid_thread: failed to initialize Sidewalk, err: -1
      [00:00:00.021,453] <inf> sid_thread: Deinitialize Sidewalk, link_mask:0
      [00:00:00.021,453] <err> sid_thread: Sidewalk stop error (code -11)
      [00:00:00.021,484] <err> sid_thread: Sidewalk deinit error (code -11)

Solution:
 Sidewalk was not provisioned.
 Clear the device, flash mfg and application again.

