//
// File: ami_stream.h (formerly part of ami.h)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
//
// $Id: ami_stream.h,v 1.1 2002-01-03 06:58:19 tavi Exp $
//
#ifndef _AMI_STREAM_H
#define _AMI_STREAM_H

#ifndef AMI_VIRTUAL_BASE
#define AMI_VIRTUAL_BASE 0
#endif

// include definition of VERSION macro
#include <versions.h>

// Include the configuration header.
#include <config.h>

// Some basic constants

// The name of the environment variable pointing to a tmp directory.
#define TMP_DIR_ENV "TMPDIR"

// The name of a tmp directory to use if the env variable is not set.
#define TMP_DIR "/var/tmp"

// Get the base class, enums, etc...
#include <ami_base.h>

// Get the device description class
#include <ami_device.h>

// Get an implementation definition

// The number of implementations to be defined.
#define _AMI_IMP_COUNT (defined(AMI_IMP_USER_DEFINED) +	\
		        defined(AMI_IMP_SINGLE))

// Multiple implementations are allowed to coexist, with some
// restrictions.  Declarations of streams must use explicit subclasses
// of AMI_base_stream to specify what type of streams they are.

// If the including module did not explicitly ask for multiple
// implementations but requested more than one implementation, issue a
// warning.
#ifndef AMI_IMP_MULTI_IMP
#if (_AMI_IMP_COUNT > 1)
#warning Multiple AMI_IMP_* defined, but AMI_IMP_MULTI_IMP undefined.
#warning Implicitly defining AMI_IMP_MULTI_IMP.
#define AMI_IMP_MULTI_IMP
#endif // (_AMI_IMP_COUNT > 1)
#endif // AMI_IMP_MULTI_IMP

// If we have multiple implementations, set AMI_STREAM to be the base
// class.
#ifdef AMI_IMP_MULTI_IMP
#define AMI_STREAM AMI_base_stream
#endif

// Now include the definitions of each implementation that will be
// used.

// Make sure at least one implementation was chosen.  If none was, then
// choose one by default, but warn the user.
#if (_AMI_IMP_COUNT < 1)
#warning No implementation defined.  Using AMI_IMP_SINGLE by default.
#define AMI_IMP_SINGLE
#endif // (_AMI_IMP_COUNT < 1)

// User defined implementation.
#if defined(AMI_IMP_USER_DEFINED)
// Do nothing.  The user will provide a definition of AMI_STREAM.
#endif // defined(AMI_IMP_USER_DEFINED)

// Single BTE stream implementation.
#if defined(AMI_IMP_SINGLE)
#include <ami_single.h>
// If this is the only implementation, then make it easier to get to.
#ifndef AMI_IMP_MULTI_IMP
#define AMI_STREAM AMI_stream_single
#endif // AMI_IMP_MULTI_IMP
#endif // defined(AMI_IMP_SINGLE)

#endif // _AMI_STREAM_H
