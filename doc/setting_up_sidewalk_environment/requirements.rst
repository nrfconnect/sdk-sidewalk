.. _sidewalk_requirements:

Requirements
############

.. contents::
   :local:
   :depth: 2

This page outlines the requirements that you need to meet before you start working with the Amazon Sidewalk environment.

.. include:: ../includes/include_sidewalk_availability.txt

Software requirements
*********************

To meet the software requirements, complete the following:

* Create an `Amazon Web Service account`_ to be able to connect AWS cloud applications with your Sidewalk Endpoints.

* Install `AWS CLI`_ with the Sidewalk command support to be able to send data to your Sidewalk Endpoints.

.. note::
   * Python3 and python3-pip are required to run the Sidewalk tools.
   * This solution was tested on Ubuntu Linux 20.04 LTS and 22.04 LTS.

.. _hardware_requirements:

Hardware requirements
*********************

Ensure you have the following:

* A supported Nordic Semiconductor's development kit and, if required, other additional hardware.
  For the list of the required devices per sample, see :ref:`samples_list`.

* Sidewalk Gateway.
  You can use one of the supported `Amazon Sidewalk Gateways`_, for example, `Echo 4th Gen`_.
  It is configured with the Alexa App on `iOS`_ or `Android`_ and requires an Amazon account for setup.

.. _requirements_memory:

RAM and flash memory requirements
*********************************

RAM and flash memory requirement values differ depending on the DK and programmed sample.

.. include:: ../includes/memory_requirement.txt
