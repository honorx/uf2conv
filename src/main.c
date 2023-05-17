/*******************************************************************************
* 说    明: Microsoft UF2 格式与 BIN 格式转换.
* 版    本: 1.0
* 作    者:
* - Microsoft Corporation <https://github.com/microsoft/uf2>
* - Dhorz <honorx@outlook.com>
* 更改记录:
* v1.0 - 初始版本
* 说    明:
*   UF2 格式与 BIN 格式转换.
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "uf2.h"

#define _DEBUG (0U)

#if (_DEBUG > 0)
  #define DEBUG(fmt, ...) printf (fmt,##__VA_ARGS__)
#else  /* if (_DEBUG > 0) */
 #define DEBUG(fmt, ...)
#endif /* if (_DEBUG > 0) */

/**
 * 短选项配置:
 * > 无冒号: 选项
 * > 单冒号: 选项带参数
 * > 双冒号: 选项带可选参数
 */
static const char const *short_option = "df:a:i:s:Fh";

/* 长选项配置 */
static const struct option long_option[] =
{
  /* *INDENT-OFF* */
  "dump",     no_argument,       NULL, 'd', /* UF2 转 BIN 模式 */
  "flags",    required_argument, NULL, 'f', /* UF2 块标志      */
  "address",  required_argument, NULL, 'a', /* 目的地址        */
  "identify", required_argument, NULL, 'i', /* 家族 ID         */
  "size",     required_argument, NULL, 's', /* 载荷大小        */
  "fixed",    no_argument,       NULL, 'F', /* 载荷大小修正    */
  "help",     no_argument,       NULL, 'h', /* 帮助信息        */
  NULL,       no_argument,       NULL, 0,
  /* *INDENT-ON* */
};

/*******************************************************************************
 * 名    称: print_usage
 * 参    数: 无
 * 返 回 值: 无
 * 描    述: 打印调试信息
 * 注意事项: 无
 ******************************************************************************/
static void print_usage (void) {
  printf ("Usage: uf2conv [--dump] [--flags=<flags>] [--address=<address>] [--identify=<identify>]\n");
  printf ("               [--size=<size>] [--fixed] [--help] <INPUT> [<OUTPUT>]\n");
  printf ("\n");
  printf ("Options:\n");
  printf ("    dump (-d)      Dump UF2 payload to binary;\n");
  printf ("    flags (-f)     Set UF2 block flags, default is 0x00000000;\n");
  printf ("    address (-a)   Set target address, default is 0x00000000; \n");
  printf ("    identify (-i)  Set family identify, default is 0x00000000;\n");
  printf ("    size (-s)      Set UF2 block payload size, default is 256 bytes;\n");
  printf ("    fixed (-F)     fixed UF2 block payload size if remaining valid data less then expected;\n");
  printf ("    help (-h)      Print usage massages;\n");
  printf ("\n");
  printf ("Example:\n");
  printf ("    uf2conv firmware.bin [firmware.uf2]\n");
  printf ("    uf2conv -d firmware.uf2 [firmware.bin]\n");
  printf ("\n");
  printf ("uf2 & bin convert tools v1.0.0 write by Dhorz <honorx@outlook.com>\n");
  printf ("\n");
}

/*******************************************************************************
 * 名    称: main
 * 参    数:
 *   argc - 参数个数
 *   argv - 参数值
 * 返 回 值: 执行状态
 * 描    述: BIN 与 UF2 格式转换
 * 注意事项: 无
 ******************************************************************************/
int main (int argc, char **argv) {
  int         res;                    /* 返回值            */
  uint32_t    dump;                   /* UF2 转 BIN        */
  uint32_t    fixed_payload_size;     /* UF2 载荷大小修正  */
  uint32_t    flags;                  /* 块标志            */
  uint32_t    target_address;         /* 目的地址          */
  uint32_t    payload_size;           /* 载荷长度          */
  uint32_t    block_no;               /* 块序号, 从 0 开始 */
  uint32_t    block_totals;           /* 块总数            */
  uint32_t    family_id;              /* 家族 ID           */
  uint32_t    file_size;              /* 文件大小          */
  uint32_t    argv_index;             /* 参数索引          */
  char        path_buffer[_MAX_PATH]; /* 输出路径缓存      */
  char       *file_path_input;        /* 输入文件路径      */
  char       *file_path_output;       /* 输出文件路径      */
  FILE       *file_input;             /* 输入文件          */
  FILE       *file_output;            /* 输出文件          */
  UF2_Block_t blk;                    /* UF2 块            */

  /* 参数初始化 */
  res                = 0;
  dump               = 0;
  fixed_payload_size = 0;
  flags              = 0x00000000UL;
  target_address     = 0x00000000UL;
  payload_size       = 256U;
  block_no           = 0;
  block_totals       = 0;
  family_id          = 0;
  file_size          = 0;
  file_path_input    = NULL;
  file_path_output   = NULL;
  file_input         = NULL;
  file_output        = NULL;

  memset (path_buffer, 0, sizeof (path_buffer));

  if (argc <= 1) {
    /* 输出使用信息 */
    print_usage ();

    res = 0;
    goto End_Of_Process;
  }

  /* 解析用户参数 */
  for ( ;; ) {
    int      idx = getopt_long (argc, argv, short_option, long_option, NULL);
    char    *stop_ptr;
    uint32_t value;

    if (idx < 0) {
      break;
    }

    switch (idx) {
    case 'd' :
      /* UF2 转 BIN 模式 */
      dump = 1;
      break;
    case 'f' :
      /* UF2 块标识 */
      errno = 0;
      value = strtoul (optarg, &stop_ptr, 0);

      if ((optarg == stop_ptr) || (errno != 0)) {
        /* 输入的不是一个数值或者是数值溢出 */
        printf ("Illegal or unknown flags !\n");

        res = -1;
        goto End_Of_Process;
      }

      flags |= value;

      break;
    case 'a' :
      /* UF2 固件起始地址 */
      errno = 0;
      value = strtoul (optarg, &stop_ptr, 0);

      if ((optarg == stop_ptr) || (errno != 0)) {
        /* 输入的不是一个数值或者是数值溢出 */
        printf ("Illegal or unknown address !\n");

        res = -1;
        goto End_Of_Process;
      }

      target_address = value;

      break;
    case 'i' :
      /* UF2 固件家族标识 */
      errno = 0;
      value = strtoul (optarg, &stop_ptr, 0);

      if ((optarg == stop_ptr) || (errno != 0)) {
        /* 输入的不是一个数值或者是数值溢出 */
        printf ("Illegal or unknown identify !\n");

        res = -1;
        goto End_Of_Process;
      }

      flags    |= UF2_FLAG_FAMILY_ID_PRESENT;
      family_id = value;

      break;
    case 's' :
      /* UF2 块载荷大小 */
      errno = 0;
      value = strtoul (optarg, &stop_ptr, 0);

      if ((optarg == stop_ptr) || (errno != 0)) {
        /* 输入的不是一个数值或者是数值溢出 */
        printf ("Illegal or unknown payload size !\n");

        res = -1;
        goto End_Of_Process;
      }

      if ((value > 476) || (value <= 0)) {
        /* 载荷大小非法 */
        printf ("Illegal payload size [0 ~ 476] !\n");

        res = -1;
        goto End_Of_Process;
      }

      payload_size = value;

      break;
    case 'F' :
      /* 载荷大小修正 */
      fixed_payload_size = 1;
      break;
    case 'h' :
      /* 输出使用信息 */
      print_usage ();
      break;
    default :
      break;
    }
  }

  argv_index = optind;

  if (argv_index < argc) {
    /* 假定第一个非选项参数是输入项 */
    file_path_input = argv[argv_index];
    argv_index++;
  } else {
    printf ("No input file !\n");

    res = -1;
    goto End_Of_Process;
  }

  file_input = fopen (file_path_input, "rb");

  if (file_input == NULL) {
    printf ("Can not open input file !\n");

    res = -1;
    goto End_Of_Process;
  }

  /* 未指定输出文件名时自动生成 */
  if (argv_index < argc) {
    /* 假定第二个非选项参数是输出项 */
    file_path_output = argv[argv_index];
    argv_index++;
  } else {
    /* 未指定输出文件名时自动生成 */
    char  drive[_MAX_DRIVE];
    char  dir[_MAX_DIR];
    char  file_name[_MAX_FNAME];
    char  ext[_MAX_EXT];
    char *str_ext;

    _splitpath (file_path_input, drive, dir, file_name, ext);

    if (dump == 0) {
      str_ext = ".uf2";
    } else {
      str_ext = ".bin";
    }

    /* 输入输出文件名相等 */
    if (strlen (ext) == 4) {
      if (memcmp (ext, str_ext, 4) == 0) {
        if (dump == 0) {
          str_ext = ".uf2.uf2";
        } else {
          str_ext = ".bin.bin";
        }
      }
    }

    _makepath (path_buffer, drive, dir, file_name, str_ext);

    file_path_output = path_buffer;
  }

  file_output = fopen (file_path_output, "wb");

  if (file_output == NULL) {
    printf ("Can not open output file !\n");

    res = -1;
    goto End_Of_Process;
  }

  fseek (file_input, 0L, SEEK_END);
  file_size = ftell (file_input);

  if (dump == 0) {
    block_totals = (file_size + (payload_size - 1)) / payload_size;

    /* UF2 块初始化 */
    memset (&blk, 0, sizeof (blk));

    blk.MagicStart0   = UF2_MAGIC_START0;
    blk.MagicStart1   = UF2_MAGIC_START1;
    blk.MagicEnd      = UF2_MAGIC_END;

    blk.Flags         = flags;
    blk.TargetAddress = target_address;
    blk.PayloadSize   = payload_size;
    blk.BlockNo       = block_no;
    blk.BlockTotals   = block_totals;
    blk.FamilyID      = family_id;

    /* BIN 转 UF2 */
    block_no = 0;
    fseek (file_input, 0L, SEEK_SET);
    fseek (file_output, 0L, SEEK_SET);

    for ( ;; ) {
      size_t len = fread (blk.Payload, 1, blk.PayloadSize, file_input);

      /* 读取文件结束 */
      if (len <= 0) {
        break;
      }

      /* 文件不足一个块时, 依据选项判断是否修正载荷长度 */
      if ((fixed_payload_size != 0) && (len < blk.PayloadSize)) {
        blk.PayloadSize = len;
      } else {
        blk.PayloadSize = payload_size;
      }

      blk.BlockNo = block_no++;

      fwrite (&blk, 1, sizeof (blk), file_output);
      memset (blk.Payload, 0, sizeof (blk.Payload));

      blk.TargetAddress += blk.PayloadSize;
    }

    fseek (file_input, 0L, SEEK_END);
    fseek (file_output, 0L, SEEK_END);

    printf ("  Convert BIN to UF2:\n");
    printf ("      Binary Size: %ld\n", ftell (file_input));
    printf ("  Family Identify: 0x%08X\n", family_id);
    printf ("   Target Address: 0x%08X\n", target_address);
    printf ("         UF2 Size: %ld\n", ftell (file_output));
    printf ("        UF2 Flags: 0x%08X\n", flags);
    printf ("   UF2 Block Size: %ld\n", payload_size);
    printf (" UF2 Block Counts: %ld\n", block_totals);
  } else {
    if ((file_size % 512) != 0) {
      printf ("Illegal UF2 file, file size must be block size align (512 bytes) !\n");

      res = -1;
      goto End_Of_Process;
    }

    /* UF2 转 BIN */
    fseek (file_input, 0L, SEEK_SET);
    fseek (file_output, 0L, SEEK_SET);

    while (fread (&blk, 1, sizeof (blk), file_input) > 0) {
      /* 判断是否有效 UF2 块 */
      if (UF2_BlockCheck (&blk) == 0) {
        printf ("Illegal UF2 file, block not UF2 block !\n");

        res = -1;
        goto End_Of_Process;
      }

      /* 判断载荷长度是否有效 */
      if ((blk.PayloadSize > 476) || (blk.PayloadSize <= 0)) {
        printf ("Illegal UF2 file, block size invalid !\n");

        res = -1;
        goto End_Of_Process;
      }

      /* 假定所有块参数一致 */
      if (blk.BlockNo == 0) {
        family_id      = blk.FamilyID;
        target_address = blk.TargetAddress;
        flags          = blk.Flags;
        payload_size   = blk.PayloadSize;
        block_totals   = blk.BlockTotals;
      }

      if (blk.TargetAddress >= target_address) {
        fseek (file_output, blk.TargetAddress - target_address, SEEK_SET);
        fwrite (blk.Payload, 1, blk.PayloadSize, file_output);
      } else {
        printf ("Illegal UF2 file, target address invalid !\n");

        res = -1;
        goto End_Of_Process;
      }
    }

    fseek (file_input, 0L, SEEK_END);
    fseek (file_output, 0L, SEEK_END);

    printf ("  Convert UF2 to BIN:\n");
    printf ("      Binary Size: %ld\n", ftell (file_input));
    printf ("  Family Identify: 0x%08X\n", family_id);
    printf ("   Target Address: 0x%08X\n", target_address);
    printf ("         UF2 Size: %ld\n", ftell (file_output));
    printf ("        UF2 Flags: 0x%08X\n", flags);
    printf ("   UF2 Block Size: %ld\n", payload_size);
    printf (" UF2 Block Counts: %ld\n", block_totals);
  }

End_Of_Process:
  if (file_input != NULL) {
    fclose (file_input);
  }

  if (file_output != NULL) {
    fclose (file_output);
  }

  return (res);
}
