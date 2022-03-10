nRF Connect SDK: sdk-sidewalk
#############################

This repository contains an Amazon Sidewalk platform abstraction layer fot the Nordic nRF Connect SDK which is based on Zephyr RTOS.

The code is in development state, it's for early evaluation and testing purposes. It shouldn't be used in production. 

The following repositories must be combined with sdk-sidewalk:

* sdk-nrf

Getting Started
***************

You can initialize this repository with west just like you use the `sdk-nrf`_  repository. You just need to replace ``sdk-nrf`` and with the ``sdk-sidewalk``, as shown in the following code:

.. parsed-literal::
   :class: highlight

   west init -m https:\ //github.com/nrfconnect/sdk-sidewalk *your-ncs-dir*
   cd *your-ncs-dir*
   west update

See :ref:`zephyr:west-basics` for more details.

Documentation
*************

Sidewalk overview:

* https://www.amazon.com/Amazon-Sidewalk/b/?node=21328123011

Official nRF Connect SDK documentation:

* http://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest
