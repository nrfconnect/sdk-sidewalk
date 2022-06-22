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

Documentation for this project is placed in ``doc`` directory. To build HTML version, execute:

.. parsed-literal::
   :class: highlight

   cd doc
   make html
   xdg-open build/html/index.html

The documentation will be placed in ``doc/build/html``. 
To see the documentation open ``doc/build/html/index.html`` in web browser.

Sidewalk overview:

* https://www.amazon.com/Amazon-Sidewalk/b/?node=21328123011

Official nRF Connect SDK documentation:

* http://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest
