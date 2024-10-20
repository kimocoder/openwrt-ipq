#
# Flow stats Makefile
#

obj-m += qca-nss-fls.o

qca-nss-fls-objs := fls_chardev.o fls_conn.o fls_debug.o fls_init.o fls_def_sensor.o fls_sensor_manager.o fls_tm.o fls_tm_chardev.o

ccflags-$(FLS_ECM_CLASSIFIER_EMESH_ENABLE) += -DFLS_ECM_CLASSIFIER_EMESH_ENABLE
ccflags-y += -Werror -Wall -Iexports/
