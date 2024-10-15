/*
 * Copyright (c) 2014 - 2015, 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/firmware/qcom/qcom_scm.h>
#include <asm/cacheflush.h>
#include <linux/dma-map-ops.h>
#include <linux/kernel_read_file.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/elf.h>
#include <linux/decompress/unlzma.h>
#include <linux/decompress/generic.h>
#include <linux/tmelcom_ipc.h>

#define QFPROM_MAX_VERSION_EXCEEDED             0x10
#define QFPROM_IS_AUTHENTICATE_CMD_RSP_SIZE	0x2

#define SW_TYPE_DEFAULT				0xFF
#define SW_TYPE_SBL				0x0
#define SW_TYPE_TZ				0x7
#define SW_TYPE_APPSBL				0x9
#define SW_TYPE_HLOS				0x17
#define SW_TYPE_RPM				0xA
#define SW_TYPE_DEVCFG				0x5
#define SW_TYPE_APDP				0x200
#define SW_TYPE_SEC_DATA			0x2B

static int gl_version_enable;
static int version_commit_enable;
static int fuse_blow_size_req;
static int decompress_error;
struct load_segs_info *ld_seg_buff;

enum qti_sec_img_auth_args {
	QTI_SEC_IMG_SW_TYPE,
	QTI_SEC_IMG_ADDR,
	QTI_SEC_HASH_ADDR,
	QTI_SEC_AUTH_ARG_MAX
};

struct qfprom_node_cfg {
	void *fuse_list;
	bool is_rlbk_support;
};

static ssize_t
qfprom_show_authenticate(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	int ret;

	if (qcom_qfprom_show_auth_available())
		ret = qcom_qfprom_show_authenticate();
	else
		ret = ipq54xx_qcom_qfprom_show_authenticate();

	if (ret < 0)
		return ret;

	/* show needs a string response */
	if (ret == 1)
		buf[0] = '1';
	else
		buf[0] = '0';

	buf[1] = '\0';

	return QFPROM_IS_AUTHENTICATE_CMD_RSP_SIZE;
}

int write_version(struct device *dev, uint32_t type, uint32_t version)
{
	int ret;
	uint32_t qfprom_ret_ptr;
	uint32_t *qfprom_api_status = kzalloc(sizeof(uint32_t), GFP_KERNEL);

	if (!qfprom_api_status)
		return -ENOMEM;

	qfprom_ret_ptr = dma_map_single(dev, qfprom_api_status,
			sizeof(*qfprom_api_status), DMA_FROM_DEVICE);

	ret = dma_mapping_error(dev, qfprom_ret_ptr);
	if (ret) {
		pr_err("DMA Mapping Error(api_status)\n");
		goto err_write;
	}

	if (qcom_qfrom_fuse_row_write_available())
		ret = qcom_qfprom_write_version(type, version, qfprom_ret_ptr);
	else
		ret = tmelcomm_secboot_update_arb_version(qfprom_ret_ptr);

	dma_unmap_single(dev, qfprom_ret_ptr,
			sizeof(*qfprom_api_status), DMA_FROM_DEVICE);

	if(ret)
		pr_err("%s: Error in QFPROM write (%d, %d)\n",
					__func__, ret, *qfprom_api_status);
	if (*qfprom_api_status == QFPROM_MAX_VERSION_EXCEEDED)
		pr_err("Version %u exceeds maximum limit. All fuses blown.\n",
							    version);

err_write:
	kfree(qfprom_api_status);
	return ret;
}

int read_version(struct device *dev, int type, uint32_t **version_ptr)
{
	int ret, ret1, ret2;
	struct qfprom_read {
		uint32_t sw_type;
		uint32_t value;
		uint32_t qfprom_ret_ptr;
	} rdip;

	uint32_t *qfprom_api_status = kzalloc(sizeof(uint32_t), GFP_KERNEL);

	if (!qfprom_api_status)
		return -ENOMEM;

	rdip.sw_type = type;
	rdip.value = dma_map_single(dev, *version_ptr,
		sizeof(uint32_t), DMA_FROM_DEVICE);

	rdip.qfprom_ret_ptr = dma_map_single(dev, qfprom_api_status,
		sizeof(*qfprom_api_status), DMA_FROM_DEVICE);

	ret1 = dma_mapping_error(dev, rdip.value);
	ret2 = dma_mapping_error(dev, rdip.qfprom_ret_ptr);

	if (ret1 == 0 && ret2 == 0) {
		ret = qcom_qfprom_read_version(type, rdip.value,
			rdip.qfprom_ret_ptr);
	}
	if (ret1 == 0) {
		dma_unmap_single(dev, rdip.value,
			sizeof(uint32_t), DMA_FROM_DEVICE);
	}
	if (ret2 == 0) {
		dma_unmap_single(dev, rdip.qfprom_ret_ptr,
			sizeof(*qfprom_api_status), DMA_FROM_DEVICE);
	}
	if (ret1 || ret2) {
		pr_err("DMA Mapping Error version ret %d api_status ret %d\n",
							ret1, ret2);
		ret = ret1 ? ret1 : ret2;
		goto err_read;
	}

	if (ret || *qfprom_api_status) {
		pr_err("%s: Error in QFPROM read (%d, %d)\n",
			 __func__, ret, *qfprom_api_status);
	}
err_read:
	kfree(qfprom_api_status);
	return ret;
}

static ssize_t generic_version(struct device *dev, const char *buf,
		uint32_t sw_type, int op, size_t count)
{
	int ret = 0;
	uint32_t *version = kzalloc(sizeof(uint32_t), GFP_KERNEL);

	if (!version)
		return -ENOMEM;

	/*
	 * Operation Type: Read: 1 and Write: 2
	 */
	switch (op) {
	case 1:
		ret = read_version(dev, sw_type, &version);
		if (ret) {
			pr_err("Error in reading version: %d\n", ret);
			goto err_generic;
		}
		ret = snprintf((char *)buf, 10, "%d\n", *version);
		break;
	case 2:
		/* Input validation handled here */
		ret = kstrtouint(buf, 0, version);
		if (ret)
			goto err_generic;

		/* Commit version is true if user input is greater than 0 */
		if (*version <= 0) {
			ret = -EINVAL;
			goto err_generic;
		}
		ret = write_version(dev, sw_type, *version);
		if (ret) {
			pr_err("Error in writing version: %d\n", ret);
			goto err_generic;
		}
		ret = count;
		break;
	default:
		ret = -EINVAL;
	}

err_generic:
	kfree(version);
	return ret;
}

#ifndef CONFIG_QCOM_TMELCOM
static ssize_t
show_sbl_version(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return generic_version(dev, buf, SW_TYPE_SBL, 1, 0);
}

static ssize_t
store_sbl_version(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	return generic_version(dev, buf, SW_TYPE_SBL, 2, count);
}

static ssize_t
show_tz_version(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return generic_version(dev, buf, SW_TYPE_TZ, 1, 0);
}

static ssize_t
store_tz_version(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	return generic_version(dev, buf, SW_TYPE_TZ, 2, count);
}

static ssize_t
show_appsbl_version(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return generic_version(dev, buf, SW_TYPE_APPSBL, 1, 0);
}

static ssize_t
store_appsbl_version(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	return generic_version(dev, buf, SW_TYPE_APPSBL, 2, count);
}

static ssize_t
show_hlos_version(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return generic_version(dev, buf, SW_TYPE_HLOS, 1, 0);
}

static ssize_t
store_hlos_version(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	return generic_version(dev, buf, SW_TYPE_HLOS, 2, count);
}

static ssize_t
show_rpm_version(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return generic_version(dev, buf, SW_TYPE_RPM, 1, 0);
}

static ssize_t
store_rpm_version(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	return generic_version(dev, buf, SW_TYPE_RPM, 2, count);
}

static ssize_t
show_devcfg_version(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return generic_version(dev, buf, SW_TYPE_DEVCFG, 1, 0);
}

static ssize_t
store_devcfg_version(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	return generic_version(dev, buf, SW_TYPE_DEVCFG, 2, count);
}

static ssize_t
show_apdp_version(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return generic_version(dev, buf, SW_TYPE_APDP, 1, 0);
}

static ssize_t
store_apdp_version(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	return generic_version(dev, buf, SW_TYPE_APDP, 2, count);
}
#else
static ssize_t
store_read_commit_version(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	int ret;
	u64 type;
	u32 version;

	if (kstrtoull(buf, 0, &type))
		return -EINVAL;

	ret = tmelcomm_secboot_get_arb_version(type, &version);
	if (ret)
		dev_err(dev, "Error in Version read for sw_type : 0x%llx\n",
			type);
	else
		dev_info(dev, "Read version for sw_type 0x%llx is : 0x%x",
			 type, version);
	return count;
}
#endif

static ssize_t
store_version_commit(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	return generic_version(dev, buf, 0, 2, count);
}

struct elf_info {
	Elf32_Off offset;
	Elf32_Word filesz;
	Elf32_Word memsize;
};

#define PT_LZMA_FLAG		0x8000000
#define PT_COMPRESS_FLAG	(PT_LOOS + PT_LZMA_FLAG + PT_LOAD)
#define IS_ELF(ehdr) ((ehdr).e_ident[EI_MAG0] == ELFMAG0 && \
			(ehdr).e_ident[EI_MAG1] == ELFMAG1 && \
			(ehdr).e_ident[EI_MAG2] == ELFMAG2 && \
			(ehdr).e_ident[EI_MAG3] == ELFMAG3)

bool is_compressed(void *header, struct elf_info *info)
{
	Elf32_Ehdr *elf_hdr = (Elf32_Ehdr*) header;
	Elf32_Phdr *prg_hdr;
	bool com_flg = false;

	if(!IS_ELF(*((Elf32_Ehdr*) header))) {
		printk("Invalid Image\n");
		return com_flg;
	}

	prg_hdr = (Elf32_Phdr *) ((unsigned long)elf_hdr + elf_hdr->e_phoff);
	for (int i = 0; i < elf_hdr->e_phnum; i++, prg_hdr++) {
		if (prg_hdr->p_type == PT_COMPRESS_FLAG) {
			com_flg = true;
			info->filesz = prg_hdr->p_filesz;
			info->offset = prg_hdr->p_offset;
			info->memsize= prg_hdr->p_memsz;
			break;
		}
	}

	return com_flg;
}

static void error(char *x)
{
        printk(KERN_ERR "%s\n", x);
	decompress_error = 1;
}


int img_decompress(void *file_buf, long size, void *out_buf, struct elf_info *info) {
	decompress_fn decompressor = NULL;
	const char *compress_name = NULL;
	int ret = -EINVAL;
	unsigned long my_inptr;
	const unsigned char *comp_data = (char *)file_buf;
	long comp_size = size;

	decompressor = decompress_method(comp_data, comp_size, &compress_name);

	if(!decompressor || !compress_name)
	{
		printk("[%s] decompress method is not configured\n", __func__);
		ret = -EINVAL;
		goto exit;
	}

	ret = decompressor((char*) comp_data, comp_size, NULL, NULL, (char*) out_buf, &my_inptr, error);
	if(decompress_error)
		ret = -EIO;
	else
		ret = 0;
exit:
	return ret;
}

static int elf64_parse_ld_segments(void *va_addr, u32 *md_size, unsigned int pa_addr)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)(uintptr_t)va_addr;
	Elf64_Phdr *phdr = (Elf64_Phdr *)(uintptr_t)(va_addr + ehdr->e_phoff);
	u8 lds_cnt = 0; /* Loadable segment count */
	u32 ld_start_addr;

	if ((!IS_ELF(*ehdr)) || ehdr->e_phnum < 3)
		return -EINVAL;
	/*
	 * Considering that first two segments of program header are not loadable segments,
	 * ignoring it for memory allocation
	 */
	ld_seg_buff = kzalloc(sizeof(*ld_seg_buff) * (ehdr->e_phnum - 2), GFP_KERNEL);
	if (!ld_seg_buff)
		return -ENOMEM;

	for (int i = 0; i < ehdr->e_phnum; i++, phdr++) {
		/* Sum of the offset and filesize of the last NULL segment */
		if (phdr->p_type == PT_NULL) {
			*md_size = phdr->p_offset + phdr->p_filesz;
			continue;
		}

		if (phdr->p_type != PT_LOAD)
			continue;

		if (!phdr->p_filesz && !phdr->p_memsz)
			continue;
		if (!phdr->p_filesz && phdr->p_memsz) {
			ld_seg_buff[lds_cnt].start_addr = 0;
			ld_seg_buff[lds_cnt++].end_addr = 0;
			continue;
		}
		/* Populating the start and end address of valid loadable
		 * segments by adding the offset with the physical elf address
		 */
		ld_start_addr = phdr->p_offset + pa_addr;
		ld_seg_buff[lds_cnt].start_addr = ld_start_addr;
		ld_seg_buff[lds_cnt++].end_addr = ld_start_addr + phdr->p_filesz;

		pr_debug("lds_cnt: %d, start_addr: %x, end_addr: %x\n", lds_cnt,
			 ld_seg_buff[lds_cnt - 1].start_addr,
			 ld_seg_buff[lds_cnt - 1].end_addr);
	}
	return lds_cnt;
}

static int elf32_parse_ld_segments(void *va_addr, u32 *md_size, unsigned int pa_addr)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)(uintptr_t)va_addr;
	Elf32_Phdr *phdr = (Elf32_Phdr *)(uintptr_t)(va_addr + ehdr->e_phoff);
	u8 lds_cnt = 0;
	u32 ld_start_addr;

	if ((!IS_ELF(*ehdr)) || ehdr->e_phnum < 3)
		return -EINVAL;
	/*
	 * Considering that first two segments of program header are not loadable segments,
	 * ignoring it for memory allocation
	 */
	ld_seg_buff = kzalloc(sizeof(*ld_seg_buff) * (ehdr->e_phnum - 2), GFP_KERNEL);
	if (!ld_seg_buff)
		return -ENOMEM;

	for (int i = 0; i < ehdr->e_phnum; i++, phdr++) {
		/* Sum of the offset and filesize of the last NULL segment */
		if (phdr->p_type == PT_NULL) {
			*md_size = phdr->p_offset + phdr->p_filesz;
			continue;
		}

		if (phdr->p_type != PT_LOAD)
			continue;

		if (!phdr->p_filesz && !phdr->p_memsz)
			continue;
		if (!phdr->p_filesz && phdr->p_memsz) {
			ld_seg_buff[lds_cnt].start_addr = 0;
			ld_seg_buff[lds_cnt++].end_addr = 0;
			continue;
		}
		/* Populating the start and end address of valid loadable
		 * segments by adding the offset with the physical elf address
		 */
		ld_start_addr = phdr->p_offset + pa_addr;
		ld_seg_buff[lds_cnt].start_addr = ld_start_addr;
		ld_seg_buff[lds_cnt++].end_addr = ld_start_addr + phdr->p_filesz;

		pr_debug("lds_cnt: %d, start_addr: %x, end_addr: %x\n", lds_cnt,
			 ld_seg_buff[lds_cnt - 1].start_addr,
			 ld_seg_buff[lds_cnt - 1].end_addr);
	}
	return lds_cnt;
}

static int parse_n_extract_ld_segment(void *va_addr, u32 *md_size, unsigned int pa_addr)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)va_addr;

	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		return elf64_parse_ld_segments(va_addr, md_size, pa_addr);
	else if (ehdr->e_ident[EI_CLASS] == ELFCLASS32)
		return elf32_parse_ld_segments(va_addr, md_size, pa_addr);
	else
		return -EINVAL;
}

static ssize_t
store_sec_auth(struct device *dev,
			struct device_attribute *sec_attr,
			const char *buf, size_t count)
{
	int ret;
	long size, sw_type;
	unsigned int img_addr, img_size, hash_size;
	char *file_name, *sec_auth_str;
	char *sec_auth_token[QTI_SEC_AUTH_ARG_MAX] = {NULL};
	static void __iomem *file_buf;
	struct device_node *np;
	int idx;
	size_t data_size = 0;
	u32 scm_cmd_id;
	void *hash_file_buf = NULL;
	void *data = NULL;
	void *out_data = NULL;
	struct elf_info info = {0};
	int ld_seg_cnt = 0;
	u32 md_size = 0; /* ELF Metadata size */
	u64 status = 0;

	file_name = kzalloc(count+1, GFP_KERNEL);
	if (file_name == NULL)
		return -ENOMEM;

	sec_auth_str = file_name;
	strlcpy(file_name, buf, count+1);

	for (idx = 0; (idx < QTI_SEC_AUTH_ARG_MAX && file_name != NULL); idx++) {
                sec_auth_token[idx] = strsep(&file_name, " ");
        }

	ret = kstrtol(sec_auth_token[QTI_SEC_IMG_SW_TYPE], 0, &sw_type);

	if (ret) {
		pr_err("sw_type str to long conversion failed\n");
		goto free_mem;
	}
	ret = kernel_read_file_from_path(sec_auth_token[QTI_SEC_IMG_ADDR],
			0, &data, INT_MAX, &data_size, READING_POLICY);

	if (ret < 0) {
		pr_err("%s file open failed\n", sec_auth_token[QTI_SEC_IMG_ADDR]);
		goto free_mem;
	}
	size = data_size;

	np = of_find_node_by_name(NULL, "qfprom");
	if (!np) {
		pr_err("Unable to find qfprom node\n");
		vfree(data);
		goto free_mem;
	}

	ret = of_property_read_u32(np, "img-size", &img_size);
	if (ret) {
		pr_err("Read of property:img-size from node failed\n");
		goto put_node;
	}

	if (size > img_size) {
		pr_err("File size exceeds allocated memory region\n");
		goto put_node;
	}

	ret = of_property_read_u32(np, "img-addr", &img_addr);
	if (ret) {
		pr_err("Read of property:img-addr from node failed\n");
		goto put_node;
	}

	ret = of_property_read_u32(np, "scm-cmd-id", &scm_cmd_id);
	if (ret)
		scm_cmd_id = QCOM_KERNEL_AUTH_CMD;

	file_buf = ioremap(img_addr, img_size);
	if (file_buf == NULL) {
		ret = -ENOMEM;
		goto put_node;
	}

	memset_io(file_buf, 0x0, img_size);

	if (data != NULL) {

		if(is_compressed(data, &info)) {
			out_data = kzalloc(info.memsize, GFP_KERNEL);
			if(!out_data) {
				pr_err("%s: Memory allocation failed for out_data buffer\n", __func__);
				goto un_map;
			}
			if(!img_decompress(data + info.offset, info.filesz, out_data, &info)) {
				printk("Uncompressed!\n");
				if(!IS_ELF(*((Elf32_Ehdr*) out_data))) {
					printk("Invalid uncompressed Image\n");
					goto free_out_data;
				} else {
					printk("uncompressed MBN extracted!\n");
					size = info.memsize;
				}
				memcpy_toio(file_buf, out_data, info.memsize);
			} else {
				printk("Failed uncompress!\n");
				goto free_out_data;
			}
		} else {
			memcpy_toio(file_buf, data, size);
		}

		vfree(data);
		data = NULL;
		data_size = 0;
	} else {
		pr_err("%s data is null\n",sec_auth_token[QTI_SEC_IMG_ADDR]);
		goto free_out_data;
	}

	if (sec_auth_token[QTI_SEC_HASH_ADDR] != NULL) {

		ret = kernel_read_file_from_path(sec_auth_token[QTI_SEC_HASH_ADDR], 0, &data,
							INT_MAX, &data_size, READING_POLICY);
		if (ret < 0) {
			pr_err("%s File open failed\n", sec_auth_token[QTI_SEC_HASH_ADDR]);
			ret = -EINVAL;
			data = NULL;
			goto free_out_data;
		}
		hash_size = data_size;
		hash_file_buf = kzalloc(hash_size, GFP_KERNEL);

		if (!hash_file_buf) {
			pr_err("%s: Memory allocation failed for hash file buffer\n", __func__);
			ret = -ENOMEM;
			goto free_out_data;
		}

		if (data != NULL) {
			memcpy(hash_file_buf, data, hash_size);
			vfree(data);
			data = NULL;
		} else {
			pr_err("%s data is null\n",sec_auth_token[QTI_SEC_HASH_ADDR]);
			goto hash_buf_alloc_err;
		}

		/*
		 * Metadata and hash contents are passed to TZ for rootfs auth
		 * for all the targets. Both signature and Hash verification is done
		 * by TZ for other targets whereas only Hash verification of rootfs
		 * is performed by TZ incase of IPQ54xx and signature verification is
		 * done by TME.
		 */
		ret = qcom_sec_upgrade_auth_meta_data(QCOM_KERNEL_META_AUTH_CMD,
						      sw_type, size, img_addr,
						      hash_file_buf, hash_size);
		if (ret) {
			pr_err("sec_upgrade_auth_meta_data failed with return=%d\n", ret);
			goto hash_buf_alloc_err;
		}

		/*
		 * Metadata is passed for signature verification of rootfs in IPQ54xx.
		 * Passing the loadable_seg_info as NULL and loadable segment count
		 * as zero for rootfs image as only the elf header is being passed.
		 */
		if (of_device_is_compatible(np, "qcom,qfprom-ipq5424-sec")) {
			ret = qcom_sec_upgrade_auth_ld_segments(scm_cmd_id, sw_type,
								img_addr, size,
								NULL, 0, &status);
			if (ret) {
				pr_err("sec_upgrade_auth_ld_segments failed with return=%d\n", ret);
				goto hash_buf_alloc_err;
			}
		}
	}
	else {
		/* Pass the file_buf containing the elf for loadable segment extraction */
		if (of_device_is_compatible(np, "qcom,qfprom-ipq5424-sec")) {
			ld_seg_cnt = parse_n_extract_ld_segment(file_buf, &md_size, img_addr);
			if (ld_seg_cnt > 0) {
				ret = qcom_sec_upgrade_auth_ld_segments(scm_cmd_id, sw_type,
									img_addr, md_size,
									ld_seg_buff, ld_seg_cnt,
									&status);
			} else {
				pr_err("Unable to parse the loadable segments: ret: %d\n",
				       ld_seg_cnt);
				goto free_out_data;
			}
		} else {
			ret = qcom_sec_upgrade_auth(scm_cmd_id, sw_type, size, img_addr);
		}
		if (ret || status) {
			pr_err("sec_upgrade_authentication failed return=%d status: %d\n",
			       ret, status);
			goto free_out_data;
		}
	}
	ret = count;

hash_buf_alloc_err:
        kfree(hash_file_buf);
free_out_data:
	if(out_data)
		kfree(out_data);
un_map:
	iounmap(file_buf);
put_node:
	of_node_put(np);
	vfree(data);
free_mem:
	kfree(sec_auth_str);
	return ret;
}

static struct device_attribute sec_attr =
	__ATTR(sec_auth, 0644, NULL, store_sec_auth);

struct kobject *sec_kobj;

static ssize_t
show_list_ipq5322_fuse(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	int ret = 0;
	int index, next = 0;
	unsigned long base_addr = 0xA00E8;
	struct fuse_payload *fuse = NULL;

	fuse = kzalloc((sizeof(struct fuse_payload) * MAX_FUSE_ADDR_SIZE),
			GFP_KERNEL);
	if (fuse == NULL) {
		return -ENOMEM;
	}

	fuse[0].fuse_addr = 0xA00D0;
	for (index = 1; index < MAX_FUSE_ADDR_SIZE; index++) {
		fuse[index].fuse_addr = base_addr + next;
		next += 0x8;
	}
	ret = qcom_scm_get_ipq_fuse_list(fuse,
			sizeof(struct fuse_payload ) * MAX_FUSE_ADDR_SIZE);
	if (ret) {
		pr_err("SCM Call failed..SCM Call return value = %d\n", ret);
		goto fuse_alloc_err;
	}

	pr_info("Fuse Name\tAddress\t\tValue\n");
	pr_info("------------------------------------------------\n");

	pr_info("TME_AUTH_EN\t0x%08X\t0x%08X\n", fuse[0].fuse_addr,
			fuse[0].lsb_val & 0x41);
	pr_info("TME_OEM_ID\t0x%08X\t0x%08X\n", fuse[0].fuse_addr,
			fuse[0].lsb_val & 0xFFFF0000);
	pr_info("TME_PRODUCT_ID\t0x%08X\t0x%08X\n", fuse[0].fuse_addr + 0x4,
			fuse[0].msb_val & 0xFFFF);

	for (index = 1; index < MAX_FUSE_ADDR_SIZE; index++) {
		pr_info("TME_MRC_HASH\t0x%08X\t0x%08X\n",
				fuse[index].fuse_addr, fuse[index].lsb_val);
		pr_info("TME_MRC_HASH\t0x%08X\t0x%08X\n",
				fuse[index].fuse_addr + 0x4, fuse[index].msb_val);
	}

fuse_alloc_err:
	kfree(fuse);
	return ret;
}

static ssize_t
show_list_ipq5424_fuse(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	int ret = 0;
	int index, next = 0;
	unsigned long base_addr = 0xA00F8;
	struct fuse_payload *fuse = NULL;

	fuse = kzalloc((sizeof(struct fuse_payload) * MAX_FUSE_ADDR_SIZE),
			GFP_KERNEL);
	if (fuse == NULL) {
		return -ENOMEM;
	}

	fuse[0].fuse_addr = 0xA00E0;
	for (index = 1; index < MAX_FUSE_ADDR_SIZE; index++) {
		fuse[index].fuse_addr = base_addr + next;
		next += 0x8;
	}
	ret = qcom_scm_get_ipq_fuse_list(fuse,
			sizeof(struct fuse_payload ) * MAX_FUSE_ADDR_SIZE);
	if (ret) {
		pr_err("SCM Call failed..SCM Call return value = %d\n", ret);
		goto fuse_alloc_err;
	}

	pr_info("Fuse Name\tAddress\t\tValue\n");
	pr_info("------------------------------------------------\n");

	pr_info("TME_AUTH_EN\t0x%08X\t0x%08X\n", fuse[0].fuse_addr,
			fuse[0].lsb_val & 0x82);
	pr_info("TME_OEM_ID\t0x%08X\t0x%08X\n", fuse[0].fuse_addr,
			fuse[0].lsb_val & 0xFFFF0000);
	pr_info("TME_PRODUCT_ID\t0x%08X\t0x%08X\n", fuse[0].fuse_addr + 0x4,
			fuse[0].msb_val & 0xFFFF);

	for (index = 1; index < MAX_FUSE_ADDR_SIZE; index++) {
		pr_info("TME_MRC_HASH\t0x%08X\t0x%08X\n",
				fuse[index].fuse_addr, fuse[index].lsb_val);
		pr_info("TME_MRC_HASH\t0x%08X\t0x%08X\n",
				fuse[index].fuse_addr + 0x4, fuse[index].msb_val);
	}

fuse_alloc_err:
	kfree(fuse);
	return ret;
}

static ssize_t
show_list_ipq9574_fuse(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	int ret = 0;
	int index = 0, next = 0;
	unsigned long base_addr = 0xA00D8;
	struct fuse_payload_ipq9574 *fuse = NULL;

	fuse = kzalloc((sizeof(struct fuse_payload_ipq9574) *
			IPQ9574_MAX_FUSE_ADDR_SIZE), GFP_KERNEL);
	if (fuse == NULL)
		return -ENOMEM;

	fuse[index++].fuse_addr = 0xA00C0;
	fuse[index].fuse_addr = 0xA00C4;

	for (index = 2; index < IPQ9574_MAX_FUSE_ADDR_SIZE; index++) {
		fuse[index].fuse_addr = base_addr + next;
		next += 0x4;
	}
	ret = qcom_scm_get_ipq_fuse_list(fuse,
				sizeof(struct fuse_payload_ipq9574) *
				IPQ9574_MAX_FUSE_ADDR_SIZE);
	if (ret) {
		pr_err("SCM Call failed..SCM Call return value = %d\n", ret);
		goto fuse_alloc_err;
	}

	pr_info("Fuse Name\tAddress\t\tValue\n");
	pr_info("------------------------------------------------\n");

	pr_info("TME_AUTH_EN\t0x%08X\t0x%08X\n", fuse[0].fuse_addr,
			fuse[0].val & 0x80);
	pr_info("TME_OEM_ID\t0x%08X\t0x%08X\n", fuse[0].fuse_addr,
			fuse[0].val & 0xFFFF0000);
	pr_info("TME_PRODUCT_ID\t0x%08X\t0x%08X\n", fuse[1].fuse_addr,
			fuse[1].val & 0xFFFF);

	for (index = 2; index < IPQ9574_MAX_FUSE_ADDR_SIZE; index++) {
		pr_info("TME_MRC_HASH\t0x%08X\t0x%08X\n",
				fuse[index].fuse_addr, fuse[index].val);
	}

fuse_alloc_err:
	kfree(fuse);
	return ret;
}

static ssize_t
store_sec_dat(struct device *dev, struct device_attribute *attr,
	      const char *buf, size_t count)
{
	int ret = count;
	loff_t size;
	u64 fuse_status = 0;
	struct file *fptr = NULL;
	struct kstat st;
	void *ptr = NULL;
	struct fuse_blow fuse_blow;
	dma_addr_t dma_req_addr = 0;
	size_t req_order = 0;
	struct page *req_page = NULL;
	u64 dma_size;
	struct device_node *np;
	u8 ld_seg_cnt = 0;
	u32 scm_cmd_id;
	u32 md_size = 0; /* ELF Metadata size */

	fptr = filp_open(buf, O_RDONLY, 0);
	if (IS_ERR(fptr)) {
		pr_err("%s File open failed\n", buf);
		ret = -EBADF;
		goto out;
	}

	np = of_find_node_by_name(NULL, "qfprom");
	if (!np) {
		pr_err("Unable to find qfprom node\n");
		goto out;
	}

	ret = vfs_getattr(&fptr->f_path, &st, STATX_SIZE, AT_STATX_SYNC_AS_STAT);
	if (ret) {
		pr_err("Getting file attributes failed\n");
		goto file_close;
	}
	size = st.size;

	/* determine the allocation order of a memory size */
	req_order = get_order(size);

	/* allocate pages from the kernel page pool */
	req_page = alloc_pages(GFP_KERNEL, req_order);
	if (!req_page) {
		ret = -ENOMEM;
		goto file_close;
	} else {
		/* get the mapped virtual address of the page */
		ptr = page_address(req_page);
	}
	memset(ptr, 0, size);
	ret = kernel_read(fptr, ptr, size, 0);
	if (ret != size) {
		pr_err("File read failed\n");
		ret = ret < 0 ? ret : -EIO;
		goto free_page;
	}

	arch_setup_dma_ops(dev, 0, 0, NULL, 0);

	dev->coherent_dma_mask = DMA_BIT_MASK(32);
	INIT_LIST_HEAD(&dev->dma_pools);
	dma_coerce_mask_and_coherent(dev, DMA_BIT_MASK(32));
	dma_size = dev->coherent_dma_mask + 1;

	/* map the memory region */
	dma_req_addr = dma_map_single(dev, ptr, size, DMA_TO_DEVICE);
	ret = dma_mapping_error(dev, dma_req_addr);
	if (ret) {
		pr_err("DMA Mapping Error\n");
		dma_unmap_single(dev, dma_req_addr, size, DMA_TO_DEVICE);
		free_pages((unsigned long)page_address(req_page), req_order);
		goto file_close;
	}
	fuse_blow.address = dma_req_addr;

	ret = of_property_read_u32(np, "scm-cmd-id", &scm_cmd_id);
	if (ret) {
		pr_err("Error in reading scm id\n");
		goto free_mem;
	}

	if (qcom_sec_dat_fuse_available()) {
		fuse_blow.status = &fuse_status;
		fuse_blow.size = fuse_blow_size_req ? size : 0;
		ret = qcom_fuseipq_scm_call(QCOM_SCM_SVC_FUSE,
					    TZ_BLOW_FUSE_SECDAT, &fuse_blow,
					    sizeof(fuse_blow));
	} else {
		/* Pass the ptr containing the elf for loadable segment extraction */
		ld_seg_cnt = parse_n_extract_ld_segment(ptr, &md_size,
							dma_req_addr);
		ret = qcom_sec_upgrade_auth_ld_segments(scm_cmd_id,
							SW_TYPE_SEC_DATA,
							dma_req_addr, md_size,
							ld_seg_buff, ld_seg_cnt,
							&fuse_status);
	}
	if (ret) {
		pr_err("Error in QFPROM write (%d)\n", ret);
		ret = -EIO;
		goto free_mem;
	}
	if (fuse_status == FUSEPROV_SECDAT_LOCK_BLOWN)
		pr_info("Fuse already blown\n");
	else if (fuse_status == FUSEPROV_INVALID_HASH)
		pr_info("Invalid sec.dat\n");
	else if (fuse_status == FUSEPROV_SUCCESS)
		pr_info("Fuse Blow Success\n");
	else
		pr_info("Fuse blow failed with err code : 0x%lx\n", fuse_status);

	ret = count;

free_mem:
	dma_unmap_single(dev, dma_req_addr, size, DMA_TO_DEVICE);
free_page:
	free_pages((unsigned long)page_address(req_page), req_order);
file_close:
	filp_close(fptr, NULL);
out:
	return ret;
}

static struct device_attribute sec_dat_attr =
	__ATTR(sec_dat, 0200, NULL, store_sec_dat);

static struct device_attribute list_ipq5322_fuse_attr =
	__ATTR(list_ipq5322_fuse, 0444, show_list_ipq5322_fuse, NULL);

static struct device_attribute list_ipq5424_fuse_attr =
	__ATTR(list_ipq5424_fuse, 0444, show_list_ipq5424_fuse, NULL);

static struct device_attribute list_ipq9574_fuse_attr =
	__ATTR(list_ipq9574_fuse, 0444, show_list_ipq9574_fuse, NULL);

static const struct qfprom_node_cfg ipq9574_qfprom_node_cfg = {
	.fuse_list		=	&list_ipq9574_fuse_attr,
	.is_rlbk_support	=	true,
};

static const struct qfprom_node_cfg ipq5332_qfprom_node_cfg = {
	.fuse_list		=	&list_ipq5322_fuse_attr,
	.is_rlbk_support	=	true,
};

static const struct qfprom_node_cfg ipq5424_qfprom_node_cfg = {
	.fuse_list		=	&list_ipq5424_fuse_attr,
	.is_rlbk_support	=	false,
};

/*
 * Do not change the order of attributes.
 * New types should be added at the end
 */
#ifdef CONFIG_QCOM_TMELCOM
static struct device_attribute qfprom_attrs[] = {
	__ATTR(authenticate, 0444, qfprom_show_authenticate, NULL),
	__ATTR(read_version, 0200, NULL, store_read_commit_version),
	__ATTR(version_commit, 0200, NULL, store_version_commit),
};
#else
static struct device_attribute qfprom_attrs[] = {
	__ATTR(authenticate, 0444, qfprom_show_authenticate,
					NULL),
	__ATTR(sbl_version, 0644, show_sbl_version,
					store_sbl_version),
	__ATTR(tz_version, 0644, show_tz_version,
					store_tz_version),
	__ATTR(appsbl_version, 0644, show_appsbl_version,
					store_appsbl_version),
	__ATTR(hlos_version, 0644, show_hlos_version,
					store_hlos_version),
	__ATTR(rpm_version, 0644, show_rpm_version,
					store_rpm_version),
	__ATTR(devcfg_version, 0644, show_devcfg_version,
					store_devcfg_version),
	__ATTR(apdp_version, 0644, show_apdp_version,
					store_apdp_version),
	__ATTR(version_commit, 0200, NULL,
					store_version_commit),

};
#endif

static struct bus_type qfprom_subsys = {
	.name = "qfprom",
	.dev_name = "qfprom",
};

static struct device device_qfprom = {
	.id = 0,
	.bus = &qfprom_subsys,
};

static int __init qfprom_create_files(int size, int16_t sw_bitmap,
				      const struct qfprom_node_cfg *cfg)
{
	int i;
	int err;
	int sw_bit;
	/* authenticate sysfs entry is mandatory */
	err = device_create_file(&device_qfprom, &qfprom_attrs[0]);
	if (err) {
		pr_err("%s: device_create_file(%s)=%d\n",
			__func__, qfprom_attrs[0].attr.name, err);
		return err;
	}

	if (cfg->is_rlbk_support && gl_version_enable != 1)
		return 0;

	for (i = 1; i < size; i++) {
		if(strncmp(qfprom_attrs[i].attr.name, "version_commit",
			   strlen(qfprom_attrs[i].attr.name))) {
			/*
			* Following is the BitMap adapted:
			* SBL:0 TZ:1 APPSBL:2 HLOS:3 RPM:4. New types should
			* be added at the end of "qfprom_attrs" variable.
			*/

			if (cfg->is_rlbk_support) {
				sw_bit = i - 1;
				if (!(sw_bitmap & (1 << sw_bit)))
					break;
			}
			err = device_create_file(&device_qfprom, &qfprom_attrs[i]);
			if (err) {
				pr_err("%s: device_create_file(%s)=%d\n",
					__func__, qfprom_attrs[i].attr.name, err);
				return err;
			}
		} else if (version_commit_enable) {
			err = device_create_file(&device_qfprom, &qfprom_attrs[i]);
			if (err) {
				pr_err("%s: device_create_file(%s)=%d\n",
					__func__, qfprom_attrs[i].attr.name, err);
				return err;
			}
		}
	}

	/* setup the DMA framework for the device 'qfprom' */
	device_qfprom.coherent_dma_mask = DMA_BIT_MASK(32);
	INIT_LIST_HEAD(&device_qfprom.dma_pools);
	dma_coerce_mask_and_coherent(&device_qfprom, DMA_BIT_MASK(32));

	arch_setup_dma_ops(&device_qfprom, 0, 0, NULL, 0);

	return 0;
}

int is_version_rlbk_enabled(struct device *dev, int16_t *sw_bitmap)
{
	int ret;
	uint32_t *version_enable = kzalloc(sizeof(uint32_t), GFP_KERNEL);
	if (!version_enable)
		return -ENOMEM;

	ret = read_version(dev, SW_TYPE_DEFAULT, &version_enable);
	if (ret) {
		pr_err("\n Version Read Failed with error %d", ret);
		goto err_ver;
	}

	*sw_bitmap = ((*version_enable & 0xFFFF0000) >> 16);

	ret = (*version_enable & 0x1);

err_ver:
	kfree(version_enable);
	return ret;
}

static int qfprom_probe(struct platform_device *pdev)
{
	int err, ret;
	int16_t sw_bitmap = 0;
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	const struct qfprom_node_cfg *cfg;
	u32 scm_cmd_id;

	if (!qcom_scm_is_available()) {
		pr_info("SCM call is not initialized, defering probe\n");
		return -EPROBE_DEFER;
	}

	cfg = of_device_get_match_data(dev);
	if (!cfg)
		return -EINVAL;

	if (cfg->is_rlbk_support) {
		gl_version_enable = is_version_rlbk_enabled(&pdev->dev, &sw_bitmap);
		if (gl_version_enable == 0)
			pr_info("\nVersion Rollback Feature Disabled\n");
	}
	/*
	 * Registering under "/sys/devices/system"
	 */
	err = subsys_system_register(&qfprom_subsys, NULL);
	if (err) {
		pr_err("%s: subsys_system_register fail (%d)\n",
			__func__, err);
		return err;
	}

	err = device_register(&device_qfprom);
	if (err) {
		pr_err("Could not register device %s, err=%d\n",
			dev_name(&device_qfprom), err);
		put_device(&device_qfprom);
		return err;
	}

	/*
	 * Registering sec_auth under "/sys/sec_authenticate"
	   only if board is secured
	 */
	if (qcom_qfprom_show_auth_available())
		ret = qcom_qfprom_show_authenticate();
	else
		ret = ipq54xx_qcom_qfprom_show_authenticate();

	if (ret < 0)
		return ret;

	if (ret == 1) {

		err = of_property_read_u32(np, "scm-cmd-id", &scm_cmd_id);
		if (err)
			scm_cmd_id = QCOM_KERNEL_AUTH_CMD;

		/*
		 * Checking if secure sysupgrade scm_call is supported
		 */
		if (!qcom_scm_sec_auth_available(scm_cmd_id)) {
			pr_info("qcom_scm_sec_auth_available is not supported\n");
		} else {
			sec_kobj = kobject_create_and_add("sec_upgrade", NULL);
			if (!sec_kobj) {
				pr_info("Failed to register sec_upgrade sysfs\n");
				return -ENOMEM;
			}

			err = sysfs_create_file(sec_kobj, &sec_attr.attr);
			if (err) {
				pr_info("Failed to register sec_auth sysfs\n");
				kobject_put(sec_kobj);
				sec_kobj = NULL;
			}
		}
	}

	of_property_read_u32(np, "fuse-blow-size-required", &fuse_blow_size_req);
	of_property_read_u32(np, "version-commit-enable", &version_commit_enable);
	if (version_commit_enable)
		pr_info("version commit support enabled\n");

	/* sysfs entry for fusing QFPROM */
	err = device_create_file(&device_qfprom, &sec_dat_attr);
	if (err) {
		pr_err("%s: device_create_file(%s)=%d\n",
			__func__, sec_dat_attr.attr.name, err);
	}

	/* Error values are printed in qfprom_create_files API. Skipping the
	   return value check to proceed with creating the next sysfs entry */
	err = qfprom_create_files(ARRAY_SIZE(qfprom_attrs), sw_bitmap, cfg);
	err = device_create_file(&device_qfprom, cfg->fuse_list);
	if (err) {
		pr_err("%s: device_create_file with error %d\n",
			__func__, err);
	}
	return err;
}

static const struct of_device_id qcom_qfprom_dt_match[] = {
	{ .compatible = "qcom,qfprom-sec",},
	{
		.compatible = "qcom,qfprom-ipq9574-sec",
		.data = (void *)&ipq9574_qfprom_node_cfg,
	}, {
		.compatible = "qcom,qfprom-ipq5332-sec",
		.data = (void *)&ipq5332_qfprom_node_cfg,
	}, {
		.compatible = "qcom,qfprom-ipq5424-sec",
		.data = (void *)&ipq5424_qfprom_node_cfg,
	},
	{}
};

static struct platform_driver qcom_qfprom_driver = {
	.driver = {
		.name	= "qcom_qfprom",
		.of_match_table = qcom_qfprom_dt_match,
	},
	.probe = qfprom_probe,
};

module_platform_driver(qcom_qfprom_driver);
