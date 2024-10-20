# Makefile for netfn-auto

ccflags-y := -I$(obj) -I$(obj)/..
ccflags-y += -Wall -Werror
ccflags-y += -DNETFN_AUTO_DEBUG_LEVEL=3

obj-m += flowmgr/
obj-m += capwapmgr/
obj-m += flowcookie/

obj ?= .
