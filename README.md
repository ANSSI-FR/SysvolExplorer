SysvolExplorer
==============

SysvolExplorer is a collection of tools designed to help security auditors to evaluate the group policy objects of an MS Active Directory architecture.

A technical study of the group policy engine has been discussed in the press article published in issue #73 of "MISC" magazine (http://www.miscmag.com/).

## SysvolCrawler 

The purpose of this software is to gather and store heterogeneous GPO information in one single place, using an easily-parsable format.

SysvolCrawler implements multiple file parsers to extract GPO data:

  * AAS files
  * ADM files
  * INF files
  * INI files
  * POL files
  * ...
  
The project also includes an LDAP client library in order to extract GPO application policy.  

SysvolCrawler outputs CSV, XML or greapable files.
  
This software has been written in C using Microsoft embedded libraries. It has been tested on Active Directory architectures from 2003 to 2012 R2 edition.

### How to use it

SysvolCrawler provides several options to customize your GPO crawling but you can give it a try using:

    SysvolCrawler.exe -d 127.0.0.1 C:\crawler\ \\127.0.0.1\sysvol\domain\policies

## SysvolBrowser

In order to assess the security of AD domains, technical auditors need a way to quickly review GPO policies. SysvolBrowser has been designed to process the huge amount of data collected with SysvolCrawler to highlight potential GPO vulnerabilities.

The development of the software is currently at an early stage and will be released as soon as possible.