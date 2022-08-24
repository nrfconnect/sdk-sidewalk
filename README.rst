nRF Connect SDK: sdk-sidewalk
#############################

This repository contains an Amazon Sidewalk platform abstraction layer fot the Nordic nRF Connect SDK which is based on Zephyr RTOS.

The code is in development state, meaning it is for early evaluation and testing purposes.
It is not used in production.

The following repositories must be combined with sdk-sidewalk:

* sdk-nrf

Requirements
************

Git LFS is required to pull Sidewalk libraries correctly.
Check the version of Git LFS installed in your OS, if there is any:

.. parsed-literal::
   :class: highlight

   git lfs version

Expected result is as follows (version number might be different):

.. parsed-literal::
   :class: highlight

   git-lfs/2.9.2 (GitHub; linux amd64; go 1.13.5)

When Git LFS is not installed, the result is as follows:

.. parsed-literal::
   :class: highlight

   git: 'lfs' is not a git command. See 'git --help'.

To install Git LFS, go to: https://git-lfs.github.com.

When Git LFS in not installed, the following build error appears:

.. parsed-literal::
   :class: highlight

   west build
   ...
   [ 95%] Linking C executable zephyr_pre0.elf
   ld.bfd: sidewalk/lib/libsid_api.a: file format not recognized; treating as linker script
   ld.bfd: sidewalk/lib/libsid_api.a:1: syntax error

Getting Started
***************

You can initialize this repository with west, similarly to the `sdk-nrf`_ repository.
To do this, you need to replace ``sdk-nrf`` with ``sdk-sidewalk``, as shown below:

.. parsed-literal::
   :class: highlight

   west init -m https:\ //github.com/nrfconnect/sdk-sidewalk *your-ncs-dir*
   cd *your-ncs-dir*
   west update

See :ref:`zephyr:west-basics` for more details.

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
