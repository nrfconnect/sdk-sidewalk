nRF Connect SDK: sdk-sidewalk
#############################

This repository contains an Amazon Sidewalk platform abstraction layer fot the Nordic nRF Connect SDK which is based on Zephyr RTOS.

The code is in development state, meaning it is for early evaluation and testing purposes.
It is not used in production.

The following repositories must be combined with sdk-sidewalk:

* sdk-nrf

Documentation
*************

Documentation for this project is placed in the ``doc`` directory.

* To preview documents in reStructuredText format, see: https://github.com/nrfconnect/sdk-sidewalk/blob/main/doc/index.rst.

* To build the HTML version, run:

  .. parsed-literal::
   :class: highlight

   cd doc
   pip install -r requirements-doc.txt
   sphinx-build -M html . build


  or run script which builds documentation in docker container (nothing except docker engine is required):


  .. parsed-literal::
   :class: highlight

   cd doc
   ./build_doc_in_docker.sh


  The documentation will be generated in ``doc/build/html``.
  To view the documentation, open ``doc/build/html/index.html`` in a web browser.

For more information about Sidewalk and nRF Connect SDK, refer to the links below:

* Sidewalk overview: https://www.amazon.com/Amazon-Sidewalk/b/?node=21328123011

* Official nRF Connect SDK documentation: http://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest
