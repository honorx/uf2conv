/*******************************************************************************
* 说    明: Microsoft UF2 格式.
* 版    本: 1.0
* 作    者:
* - Microsoft Corporation <https://github.com/microsoft/uf2>
* - Dhorz <honorx@outlook.com>
* 更改记录:
* v1.0 - 初始版本
* 说    明:
*   UF2 是由 Microsoft 开发的文件格式, 适用于通过 USB MSC 更新微控制器固件. UF2
*  由 512 字节块组成, 包含块头, 数据, 块尾三个部分; 通过 MSC 传输的属于始终以
*  512 字节的倍数传输, 所以很容易识别 UF2 块. UF2 主要解决如下问题:
*  <*> 操作系统 (OS) 写入块的顺序与文件中出现的顺序不同.
*  <*> 操作系统写入多次阻塞.
*  <*> 操作系统写入的数据不是 UF2 块.
*  <*> 操作系统写入块的第一部分/最后一部分，可能用于元数据检测或搜索索引.
*******************************************************************************/
#ifndef UF2_FORMAT_H
#define UF2_FORMAT_H 0x00000100

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* 所有条目都是小端模式 */
#define UF2_MAGIC_START0           (0x0A324655UL) /* 块头 0: "UF2\n"    */
#define UF2_MAGIC_START1           (0x9E5D5157UL) /* 块头 1: 随机选取值 */
#define UF2_MAGIC_END              (0x0AB16F30UL) /* 块尾 0：随机选取值 */

#define UF2_FLAG_NO_FLASH          (0x00000001UL) /* BLK 无需写入到设备   */
#define UF2_FLAG_FILE_CONTAINER    (0x00001000UL) /* BLK 是文件容器       */
#define UF2_FLAG_FAMILY_ID_PRESENT (0x00002000UL) /* BLK FAMILY_ID 域有效 */
#define UF2_FLAG_MD5_PRESENT       (0x00004000UL) /* BLK 包含 MD5 校验    */
#define UF2_FLAG_EXT_TAGS_PRESENT  (0x00008000UL) /* BLK 包含扩展标志     */

/* UF2 块: 32 字节块头 + 476 字节载荷 (通常使用 256 字节) + 4 字节块尾 */
typedef struct {
  uint32_t MagicStart0;   /* 块头标志 0        */
  uint32_t MagicStart1;   /* 块头标志 1        */
  uint32_t Flags;         /* 块标志            */
  uint32_t TargetAddress; /* 目的地址          */
  uint32_t PayloadSize;   /* 载荷长度          */
  uint32_t BlockNo;       /* 块序号, 从 0 开始 */
  uint32_t BlockTotals;   /* 块总数            */
  uint32_t FamilyID;      /* 家族 ID           */
  uint8_t  Payload[476];  /* 载荷数据          */
  uint32_t MagicEnd;      /* 块尾标志          */
} UF2_Block_t;

/* 检查数据块是否是 UF2 块, 有效返回非 0 值 */
static inline int8_t UF2_BlockCheck (const void *pData) {
  const UF2_Block_t *blk = (const UF2_Block_t *)pData;

  return ((blk->MagicStart0 == UF2_MAGIC_START0) && (blk->MagicStart1 == UF2_MAGIC_START1) && (blk->MagicEnd == UF2_MAGIC_END));
}

/* 检查家族 ID 是否匹配, 匹配返回非 0 值 */
static inline int8_t UF2_FamilyCheck (const void *pData, uint32_t Family) {
  const UF2_Block_t *blk = (const UF2_Block_t *)pData;

  return (((blk->Flags & UF2_FLAG_FAMILY_ID_PRESENT) != 0) && (blk->FamilyID == Family));
}

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef UF2_FORMAT_H */
