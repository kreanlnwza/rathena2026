---
name: rAthena Development
description: คู่มือสำหรับการพัฒนา rAthena MMORPG Server Emulator รวมถึงการเขียน NPC script, แก้ไข C++ source code, และการตั้งค่า server
การตอบคำถามให้ตอบเป็นภาษาไทย
---

# rAthena Development Skill

## ภาพรวมโปรเจค

rAthena เป็น Ragnarok Online Private Server Emulator เขียนด้วย C++ ประกอบด้วย:

- **Login Server** - จัดการ authentication
- **Char Server** - จัดการข้อมูลตัวละคร
- **Map Server** - จัดการ game logic หลัก
- **Web Server** - API สำหรับ web services

## โครงสร้างโฟลเดอร์

```
rAthena/
├── src/              # C++ source code
│   ├── common/       # shared utilities และ libraries
│   ├── login/        # login server
│   ├── char/         # character server
│   ├── map/          # map server (NPC, items, skills, mobs)
│   ├── web/          # web server (REST API)
│   └── config/       # compile-time configuration
├── conf/             # runtime configuration files
├── db/               # database files (YAML format)
├── npc/              # NPC scripts
├── sql-files/        # MySQL database schemas
├── doc/              # documentation
└── tools/            # utility scripts
```

## การพัฒนา NPC Script

### รูปแบบหัวเอกสาร (Doc / Note Header)

**⚠️ สำคัญ:** ทุกไฟล์ NPC Script และ SRC ที่สร้างหรือแก้ไขต้องมีหัวเอกสาร (documentation header) ที่เป็นมาตรฐาน

**ชื่อเจ้าของ (Author Credit):**
```
V!be Coding [kreanlnwza] AI Assistant (ชื่อ Ai แต่ละ model)
```

#### NPC Script Header (ไฟล์ `.txt` ใน `npc/`)

```c
//===== rAthena Script =======================================
//= ชื่อ Script
//===== Description: =========================================
//= คำอธิบายว่า script ทำอะไร
//= บรรทัดที่ 2 (ถ้ามี)
//=
//= NOTE: หมายเหตุสำคัญ (ถ้ามี)
//===== Changelogs: ==========================================
//= 1.0 First version [ชื่อผู้สร้าง]
//= 1.1 รายละเอียดการแก้ไข [ชื่อผู้แก้ไข]
//= x.x Maintained by V!be Coding [kreanlnwza] AI Assistant (Antigravity)
//============================================================
```

**ตัวอย่างจริง** (จาก `npc/custom/card_seller.txt`):
```c
//===== rAthena Script =======================================
//= Card Seller A-Z
//===== Description: =========================================
//= Sells all cards dropped by mobs, grouped alphabetically.
//= MVP cards are excluded from the list.
//=
//= NOTE: Requires SQL item and mob databases.
//===== Changelogs: ==========================================
//= 1.0 First version [AnnieRuru]
//= 1.1 Minor edits [Euphy]
//= 1.2 Update for monster mode and enchants [Lemongrass]
//============================================================
```

#### C++ Source Header (ไฟล์ `.cpp` / `.hpp` / `.inc`)

สำหรับไฟล์ `src/custom/*.inc` และไฟล์ C++ ที่สร้างใหม่:

```cpp
/**
 * ชื่อ/คำอธิบายสั้น
 * รายละเอียดเพิ่มเติม
 * Author: V!be Coding [kreanlnwza] AI Assistant (Antigravity)
 *
 * NOTE: หมายเหตุสำคัญ (ถ้ามี)
 **/
```

**สำหรับ function-level documentation:**
```cpp
/**
 * ชื่อฟังก์ชัน - คำอธิบาย
 * @param param1 คำอธิบาย parameter
 * @return คำอธิบาย return value
 * @author V!be Coding [kreanlnwza] AI Assistant (Antigravity)
 */
```

#### กฎการใส่ doc/note header

1. **ไฟล์ NPC ใหม่** - ต้องมี header ครบทุกส่วน (Title, Description, Changelogs)
2. **ไฟล์ NPC ที่แก้ไข** - เพิ่ม changelog บรรทัดใหม่ พร้อม version + ชื่อ
3. **ไฟล์ SRC ใหม่** - ต้องมี `/**` doc block พร้อม Author
4. **ไฟล์ SRC ที่แก้ไข** - เพิ่ม `@author` ในฟังก์ชันที่เพิ่ม/แก้ไข
5. **ห้ามแก้ไข** header เดิมของผู้เขียนต้นฉบับ (เช่น `[AnnieRuru]`, `[Euphy]`)

### ไฟล์ที่เกี่ยวข้อง
- `npc/` - NPC script files
- `doc/script_commands.txt` - คำสั่ง script ทั้งหมด (สำคัญมาก!)
- `doc/sample/` - ตัวอย่าง script

### เครื่องมือที่แนะนำ

**rAthena Language Support** (VS Code Extension)
- Syntax highlighting สำหรับ rAthena scripting language
- Code snippets สำหรับ NPC scripts
- ติดตั้ง: ค้นหา "rAthena" ใน VS Code Extensions

### รูปแบบพื้นฐาน NPC Script

#### 1. NPC ทั่วไป (Map NPC)
```c
// รูปแบบ: map,x,y,direction	script	NPC_Name	sprite_id,trigger_x,trigger_y,{
prontera,155,180,4	script	Sample NPC	100,{
    mes "[Sample NPC]";
    mes "Hello!";
    next;
    switch(select("Option 1:Option 2")) {
        case 1:
            mes "เลือก 1";
            break;
        case 2:
            mes "เลือก 2";
            break;
    }
    close;
}
```

**พารามิเตอร์:**
| พารามิเตอร์ | คำอธิบาย |
|-------------|----------|
| `map` | ชื่อแมพ เช่น `prontera`, `geffen` |
| `x,y` | พิกัดบนแมพ |
| `direction` | ทิศหัน: 0=เหนือ, 2=ตะวันตก, 4=ใต้, 6=ตะวันออก |
| `script` | ประเภท (script, warp, shop, cashshop, duplicate) |
| `NPC_Name` | ชื่อ NPC (ใช้ `#suffix` สำหรับ unique name) |
| `sprite_id` | รหัสรูปร่าง NPC (-1 = ไม่มีตัว/floating) |
| `trigger_x,y` | ขนาดพื้นที่ trigger (ไม่บังคับ) |

#### 2. Floating NPC (ไม่มีตำแหน่งบนแมพ)
```c
-	script	MyFloatingNPC	-1,{
    // ใช้สำหรับ event handler, background process
    end;
OnInit:
    // โค้ดที่รันตอน server start
    end;
}
```

#### 3. Warp NPC
```c
// รูปแบบ: map,x,y,0	warp	WarpName	trigger_x,trigger_y,dest_map,dest_x,dest_y
prontera,155,22,0	warp	prt_exit	2,2,prt_fild08,170,375
```

#### 4. Shop NPC
```c
// รูปแบบ: map,x,y,dir	shop	ShopName	sprite_id,item_id:price,...
prontera,150,180,4	shop	ร้านค้า	100,501:100,502:200,503:500
```

#### 5. Cash Shop NPC
```c
// รูปแบบ: map,x,y,dir	cashshop	ShopName	sprite_id,item_id:price,...
prontera,150,180,4	cashshop	ร้าน Cash	100,12103:500,12104:300
```

#### 6. NPC พร้อม OnTouch (trigger area)
```c
// trigger_x,trigger_y ระบุพื้นที่กระตุ้น
prontera,155,180,4	script	Area NPC	100,5,5,{
    mes "คุณเข้ามาในพื้นที่!";
    close;
OnTouch:
    // เมื่อผู้เล่นเดินเข้าพื้นที่ 5x5
    mes "ยินดีต้อนรับ!";
    close;
}
```

#### 7. NPC Trader (ร้านค้าแบบกำหนดเอง)
```c
-	trader	MyTrader	-1,{
OnInit:
    tradertype(NST_ZENY);
    sellitem 501, 100;   // Red Potion ราคา 100
    sellitem 502, 200;   // Orange Potion ราคา 200
    end;
OnCountFunds:
    traderreadfunds;
    end;
OnPayFunds:
    traderpayresult(1);
    end;
}

// Duplicate ไปวางบนแมพ
prontera,150,180,4	duplicate(MyTrader)	ร้านค้า#prt	100
```

#### การจบ Script
| คำสั่ง | คำอธิบาย |
|--------|----------|
| `close;` | ปิด dialog (ต้องมี player attached) |
| `close2;` | ปิด dialog แล้วรัน script ต่อ |
| `end;` | จบ script ทันที (ไม่ต้องมี player) |
| `next;` | แสดงปุ่ม Next ให้ผู้เล่นกด |

### คำสั่ง Script ที่ใช้บ่อย

| คำสั่ง | คำอธิบาย |
|--------|----------|
| `mes "text"` | แสดงข้อความ |
| `next` | รอการกดปุ่ม next |
| `close` | ปิดหน้าต่าง NPC |
| `select("opt1:opt2")` | แสดงตัวเลือก |
| `getitem <id>,<amount>` | ให้ไอเทม |
| `delitem <id>,<amount>` | ลบไอเทม |
| `Zeny` | ตัวแปร Zeny ของผู้เล่น |
| `#CASHPOINTS` | ตัวแปร Cash Point ของผู้เล่น |
| `#KAFRAPOINTS` | ตัวแปร Kafra Point ของผู้เล่น |
| `countitem(<id>)` | นับจำนวนไอเทม |
| `BaseLevel` | เลเวลของผู้เล่น |
| `strcharinfo(0)` | ชื่อตัวละคร |
| `getitemname(<id>)` | ชื่อไอเทมจาก ID |

### Best Practices: การใช้ตัวแปร

**⚠️ สำคัญ:** ใช้ตัวแปรหรือ constants แทนการใส่ตัวเลขตรงๆ เสมอ!

**❌ ไม่ดี:**
```c
if (countitem(12580) >= 2) {
    delitem 12580, 2;
    getitem 7227, 1;
}
```

**✅ ดี:**
```c
.@required_item = 12580;  // Token of Siegfried
.@reward_item = 7227;      // Silver Coin
.@required_amount = 2;

if (countitem(.@required_item) >= .@required_amount) {
    delitem .@required_item, .@required_amount;
    getitem .@reward_item, 1;
}
```

### การแสดงรูปไอเทมพร้อมชื่อ

ใช้ `^i[ItemID]` เพื่อแสดงรูปไอเทม และ `getitemname()` เพื่อแสดงชื่อ:

```c
// แสดงรูปไอเทม + ชื่อ + จำนวน
mes "- ^i[12580] " + getitemname(12580) + " x2";

// ใช้กับตัวแปร (แนะนำ)
.@item_id = 12580;
.@amount = 2;
mes "- ^i[" + .@item_id + "] " + getitemname(.@item_id) + " x" + .@amount;
```

**ผลลัพธ์:** จะแสดง [รูปไอเทม] Token of Siegfried x2

| รูปแบบ | คำอธิบาย |
|--------|----------|
| `^i[ItemID]` | แสดงรูป icon ของไอเทม |
| `^e[EmotionID]` | แสดง emoticon/อีโมจิ |
| `getitemname(ItemID)` | แสดงชื่อไอเทม |

### Emoticon สำหรับ NPC (^e[])

ใช้ `^e[EmotionID]` เพื่อแสดง emoticon ใน dialog:

```c
mes "ยินดีต้อนรับ! ^e[1]";   // แสดง emoticon หมายเลข 1
mes "ขอบคุณมาก ^e[29]";      // หัวใจ
mes "อย่าลืมนะ! ^e[5]";      // ตกใจ
```

**Emoticon ID ที่ใช้บ่อย:**

| ID | Emoticon | คำอธิบาย |
|----|----------|----------|
| 0 | /! | ตกใจ |
| 1 | /? | สงสัย |
| 2 | /ho | ดีใจ |
| 5 | /omg | OMG/ตกใจ |
| 20 | /... | คิด |
| 29 | /lv | หัวใจ |
| 30 | /swt | เหงื่อตก |

> 💡 ดูรายการ Emoticon ทั้งหมดได้ที่ `doc/effect_list.md`

### Color Codes (สีข้อความ)

ใช้ `^RRGGBB` เพื่อเปลี่ยนสีข้อความ:

```c
mes "^FF0000สีแดง^000000";
mes "^00FF00สีเขียว^000000";
mes "^0000FFสีน้ำเงิน^000000";
mes "^FFD700สีทอง^000000";
mes "^FF69B4สีชมพู^000000";
```

**หมายเหตุ:** ต้องปิดด้วย `^000000` (สีดำ) เพื่อรีเซ็ตสี

### ประเภทตัวแปร NPC Script

| ตัวแปร | Scope | คงอยู่ | ตัวอย่าง |
|--------|-------|--------|----------|
| `.@var` | Local | หายเมื่อจบ script | `.@i = 1;` |
| `@var` | Player attached | หายเมื่อ logout | `@points = 100;` |
| `$var` | Global server | ค้างถาวร | `$event_on = 1;` |
| `$@var` | Global array | ค้างถาวร | `$@names$[0]` |
| `.var` | NPC scope | ค้างถึง restart | `.count = 0;` |
| `#var` | Character permanent | บันทึกใน DB | `#quest_done` |
| `##var` | Account permanent | ทุก char ใช้ร่วม | `##vip_days` |

**หมายเหตุ:** เพิ่ม `$` ท้ายตัวแปรสำหรับ string เช่น `.@name$`

### Timer & Sleep

```c
// หยุด script ชั่วคราว
sleep 1000;           // หยุด 1 วินาที (block player)
sleep2 1000;          // หยุดแต่ไม่ block player

// ตั้ง timer เรียก label
addtimer 5000, "NPC::OnTimer";      // เรียกหลัง 5 วินาที
deltimer "NPC::OnTimer";            // ยกเลิก timer

// ตัวอย่าง timer NPC
-	script	TimerNPC	-1,{
OnTimer:
    announce "5 วินาทีผ่านไป!", bc_all;
    end;
}
```

### Query SQL

```c
// SELECT
query_sql("SELECT `account_id`, `userid` FROM `login` WHERE `account_id` = " + getcharid(3), .@aid, .@user$);
mes "Account: " + .@user$;

// INSERT/UPDATE/DELETE
query_sql("UPDATE `login` SET `lastlogin` = NOW() WHERE `account_id` = " + getcharid(3));
```

**⚠️ ระวัง SQL Injection!** ใช้ `escape_sql()` สำหรับ user input

### Array Functions

```c
// สร้าง array
setarray .@items[0], 501, 502, 503, 504, 505;

// ขนาด array
.@size = getarraysize(.@items);

// วนลูป array
for (.@i = 0; .@i < .@size; .@i++) {
    mes "Item: " + .@items[.@i];
}

// copy array
copyarray .@copy[0], .@items[0], getarraysize(.@items);

// ลบ element
deletearray .@items[2], 1;  // ลบ index 2

// clear array
cleararray .@items[0], 0, getarraysize(.@items);
```

### Party & Guild Functions

```c
// ตรวจสอบ party
if (getcharid(1) == 0) {
    mes "คุณไม่มี Party!";
    close;
}

// ดึงสมาชิก party
getpartymember getcharid(1), 0;  // เก็บชื่อใน $@partymembername$
getpartymember getcharid(1), 1;  // เก็บ char id ใน $@partymembercid
getpartymember getcharid(1), 2;  // เก็บ account id ใน $@partymemberaid
.@count = $@partymembercount;

// ตรวจสอบ guild
if (getcharid(2) == 0) {
    mes "คุณไม่มี Guild!";
    close;
}
```

### Instance / Dungeon System

```c
// สร้าง instance
.@instance_id = instance_create("Memorial Dungeon", getcharid(1), IOT_PARTY);

if (.@instance_id < 0) {
    mes "ไม่สามารถสร้าง Instance ได้!";
    close;
}

// attach และ warp
instance_attach(.@instance_id);
warp "1@tower", 50, 50;

// ตั้งเวลา
instance_set_timeout 3600, 300, .@instance_id;  // 1 ชม., 5 นาที warning

// ทำลาย
instance_destroy .@instance_id;
```

### Common Event Labels

| Label | เหตุการณ์ | หมายเหตุ |
|-------|-----------|----------|
| `OnInit` | Server start | โหลดครั้งแรก |
| `OnPCLoginEvent` | Player login | - |
| `OnPCLogoutEvent` | Player logout | - |
| `OnPCDieEvent` | Player ตาย | - |
| `OnPCKillEvent` | Player ฆ่า player | - |
| `OnNPCKillEvent` | Player ฆ่า monster | - |
| `OnPCBaseLvUpEvent` | Base level up | - |
| `OnPCJobLvUpEvent` | Job level up | - |
| `OnPCStatCalcEvent` | คำนวณ stat | - |
| `OnWhisperGlobal` | ได้รับ whisper | - |

```c
// ตัวอย่าง Event NPC
-	script	LoginBonus	-1,{
OnPCLoginEvent:
    if (#login_bonus < gettimetick(2)) {
        getitem 501, 10;  // Red Potion x10
        #login_bonus = gettimetick(2) + 86400;  // พรุ่งนี้
        dispbottom "รับโบนัสล็อกอินวันนี้แล้ว!";
    }
    end;
```

### callfunc และ Global Functions

ใช้ `callfunc` เพื่อเรียกใช้ global functions ที่อยู่ใน `npc/other/Global_Functions.txt`:

#### F_InsertComma - Format ตัวเลขเป็น comma separated

```c
.@zeny = 7777777;
mes "ราคา: " + callfunc("F_InsertComma", .@zeny) + " Zeny";
// ผลลัพธ์: "ราคา: 7,777,777 Zeny"

// ใช้ร่วมกับการแสดงไอเทม
.@cost = 1500000;
mes "ต้องใช้ ^FF0000" + callfunc("F_InsertComma", .@cost) + " Zeny^000000";
```

#### Global Functions ที่มีประโยชน์

| Function | คำอธิบาย | ตัวอย่าง |
|----------|----------|----------|
| `F_InsertComma` | เพิ่ม comma ในตัวเลข | `callfunc("F_InsertComma", 1000000)` → `"1,000,000"` |
| `F_InsertPlural` | พหูพจน์อัตโนมัติ | `callfunc("F_InsertPlural", 5, "item")` → `"5 items"` |
| `F_Rand` | สุ่มจาก arguments | `callfunc("F_Rand", "A", "B", "C")` → random |
| `F_GetNumSuffix` | เพิ่ม st/nd/rd/th | `callfunc("F_GetNumSuffix", 1)` → `"1st"` |
| `Time2Str` | แปลงเวลาเป็นข้อความ | `callfunc("Time2Str", .@time)` → `"1 day, 2 hours"` |
| `F_GetWeaponType` | ชื่อประเภทอาวุธ | `callfunc("F_GetWeaponType", 1201)` → `"Dagger"` |

#### ตัวอย่างการใช้งานร่วมกัน

```c
// NPC ร้านค้าที่แสดงราคาสวยๆ
.@price = 2500000;
mes "^i[501] " + getitemname(501) + " x10";
mes "ราคา: ^FF0000" + callfunc("F_InsertComma", .@price) + "^000000 Zeny";

if (Zeny < .@price) {
    mes "คุณมี Zeny ไม่พอ! (" + callfunc("F_InsertComma", Zeny) + ")";
    close;
}
```

> 📖 **อ้างอิง:** ดูคำสั่ง script ทั้งหมดได้ที่ `doc/script_commands.txt`
> 📖 **Global Functions:** `npc/other/Global_Functions.txt`

### การเพิ่ม NPC ใหม่

1. สร้างไฟล์ในโฟลเดอร์ `npc/custom/`
2. เพิ่ม path ในไฟล์ `npc/scripts_custom.conf`
3. รีโหลดด้วย `@reloadscript` ใน game

### NPC Duplicate (โคลน NPC)

ใช้สำหรับสร้าง NPC หลายตัวที่ใช้โค้ดเดียวกัน:

```c
// NPC หลัก (floating NPC ไม่มีตำแหน่ง)
-	script	MyNPC	-1,{
    mes "[NPC]";
    mes "Hello!";
    close;
}

// Duplicates - โคลนไปหลายตำแหน่ง
prontera,150,180,4	duplicate(MyNPC)	MyNPC#prt	100
geffen,120,60,4	duplicate(MyNPC)	MyNPC#gef	100
payon,180,100,4	duplicate(MyNPC)	MyNPC#pay	100
```

**รูปแบบ:**
```
map,x,y,dir	duplicate(OriginalNPC)	UniqueName	sprite_id
```

### การใช้ Menu และ Switch

**วิธีที่ 1: switch + select (แนะนำ)**
```c
switch(select("ตัวเลือก 1:ตัวเลือก 2:ยกเลิก")) {
    case 1:
        mes "เลือกตัวเลือก 1";
        break;
    case 2:
        mes "เลือกตัวเลือก 2";
        break;
    case 3:
        mes "ยกเลิก";
        break;
}
```

**วิธีที่ 2: menu + goto**
```c
menu "ตัวเลือก 1",L_Option1,"ตัวเลือก 2",L_Option2,"ยกเลิก",L_Cancel;

L_Option1:
    mes "เลือกตัวเลือก 1";
    close;

L_Option2:
    mes "เลือกตัวเลือก 2";
    close;

L_Cancel:
    mes "ยกเลิก";
    close;
```

**วิธีที่ 3: prompt (ไม่ยกเลิกได้)**
```c
// prompt คล้าย select แต่กด ESC จะ return 255
.@choice = prompt("ตัวเลือก 1:ตัวเลือก 2");
if (.@choice == 255) {
    mes "ยกเลิก";
    close;
}
```

**เปรียบเทียบ:**
| คำสั่ง | ESC/Cancel | Return Value |
|--------|------------|--------------|
| `select()` | ไม่ได้ | 1, 2, 3... |
| `prompt()` | return 255 | 1, 2, 3... หรือ 255 |
| `menu` | ไม่ได้ | goto label |

### NPC Dialog Formatting

#### การจัดตำแหน่ง Dialog

**setdialogalign(<align>)** - กำหนดการจัดตำแหน่งข้อความใน NPC dialog

| Horizontal Align | คำอธิบาย |
|------------------|----------|
| `DIALOG_ALIGN_LEFT` | ชิดซ้าย |
| `DIALOG_ALIGN_CENTER` | กึ่งกลาง |
| `DIALOG_ALIGN_RIGHT` | ชิดขวา |

| Vertical Align | คำอธิบาย |
|----------------|----------|
| `DIALOG_ALIGN_TOP` | อยู่บน |
| `DIALOG_ALIGN_MIDDLE` | ตรงกลาง |
| `DIALOG_ALIGN_BOTTOM` | อยู่ล่าง |

#### คำสั่งจัดการขนาดและตำแหน่ง Dialog

| คำสั่ง | คำอธิบาย |
|--------|----------|
| `setdialogsize(<width>,<height>)` | กำหนดขนาดหน้าต่าง dialog (pixel) |
| `setdialogpos(<x>,<y>)` | กำหนดตำแหน่งหน้าต่าง dialog (pixel) |
| `setdialogpospercent(<x>,<y>)` | กำหนดตำแหน่งหน้าต่าง dialog (% ของหน้าจอ, 0-100) |

#### HTML Formatting สำหรับ mes

สามารถใช้ HTML tags ใน `mes` ได้:

| Tag | ตัวอย่าง | ผลลัพธ์ |
|-----|----------|---------|
| `<B>` | `<B>ตัวหนา</B>` | **ตัวหนา** |
| `<FONT SIZE=n>` | `<FONT SIZE=15>ใหญ่</FONT>` | ข้อความขนาด 15 |
| `<FONT COLOR=#RRGGBB>` | `<FONT COLOR=#FF0000>แดง</FONT>` | ข้อความสีแดง |

#### ตัวอย่างการใช้งาน

```c
prontera,100,100,3	script	ตัวอย่าง NPC	858,2,2,{
    mes "<FONT SIZE=15><B>หัวข้อหลัก</B></FONT>";
    mes "<FONT SIZE=12>รายละเอียด</FONT>";
    mes " ";
    mes "ข้อความปกติ";
    setdialogalign(DIALOG_ALIGN_CENTER);
    setdialogalign(DIALOG_ALIGN_MIDDLE);
    setdialogsize(450,150);
    setdialogpos(575,375);
    close;
}
```

**ตัวอย่างการใช้ setdialogpospercent (กึ่งกลางหน้าจอ):**
```c
prontera,100,100,3	script	Center Dialog	858,{
    mes "ข้อความกึ่งกลางหน้าจอ";
    setdialogalign(DIALOG_ALIGN_CENTER);
    setdialogalign(DIALOG_ALIGN_MIDDLE);
    setdialogsize(400,120);
    setdialogpospercent(50,50);  // กึ่งกลางหน้าจอ (50%, 50%)
    close;
}
```

**หมายเหตุ:**
- `setdialogalign()` ต้องเรียกก่อน `close;` หรือ `next;`
- เรียก horizontal และ vertical align แยกกัน
- `mes " ";` ใช้สำหรับเว้นบรรทัดว่าง
- `setdialogpospercent` ใช้ค่า 0-100 (เปอร์เซ็นต์ของหน้าจอ) ต่างจาก `setdialogpos` ที่ใช้ pixel

## การพัฒนา C++ Source Code

### Build System

- ใช้ **Visual Studio** บน Windows (rAthena.sln)
- ใช้ **CMake** หรือ **Make** บน Linux

### ไฟล์สำคัญ

| Path | คำอธิบาย |
|------|----------|
| `src/map/script.cpp` | Script engine และ buildin functions |
| `src/map/skill.cpp` | ระบบ skill |
| `src/map/mob.cpp` | AI และ behavior ของ monster |
| `src/map/pc.cpp` | Player character functions |
| `src/map/npc.cpp` | NPC system |
| `src/map/battle.cpp` | ระบบ battle calculation |
| `src/map/atcommand.cpp` | @ commands |
| `src/common/` | Shared utilities |

### การเพิ่ม Script Command ใหม่

1. เปิด `src/map/script.cpp`
2. สร้างฟังก์ชัน `BUILDIN_FUNC(command_name)`
3. ลงทะเบียนใน `script_def_buildin[]`
4. Compile และ restart server

```cpp
// ตัวอย่าง buildin function
BUILDIN_FUNC(mycommand)
{
    map_session_data* sd;
    if (!script_rid2sd(sd))
        return SCRIPT_CMD_FAILURE;
    
    int value = script_getnum(st, 2);
    // ทำงานที่ต้องการ
    
    script_pushint(st, 1); // return value
    return SCRIPT_CMD_SUCCESS;
}
```

### Coding Standards

- ใช้ tabs สำหรับ indentation (ห้ามลบ!)
- ไม่แก้ไขข้อความเดิมโดยไม่ได้รับอนุญาต
- ตั้งชื่อตัวแปรเป็น snake_case
- **⚠️ NOTE: สำหรับการใช้ `int` ในทุกไฟล์ ต้องเป็น `int32` เสมอ**
- **⚠️ NOTE: วางแผนและ task งานก่อนทำงานเสมอ ก่อนทำงานจริง **

### map_session_data และ block_list

**⚠️ สำคัญ:** `map_session_data` สืบทอด (inherit) มาจาก `block_list` โดยตรง ดังนั้น:

| ❌ ผิด | ✅ ถูก |
|--------|--------|
| `sd->bl.m` | `sd->m` |
| `sd->bl.x` | `sd->x` |
| `sd->bl.y` | `sd->y` |
| `sd->bl.id` | `sd->id` |
| `&sd->bl` | `sd` (ใช้ pointer โดยตรง) |

```cpp
// ❌ ผิด - จะเกิด error: 'bl' is not a member of 'map_session_data'
if (sd->bl.m == some_map)

// ✅ ถูก - เข้าถึง member โดยตรง
if (sd->m == some_map)

// ❌ ผิด - ส่ง &sd->bl
status_calc_bl(&sd->bl, { SCB_SPEED });

// ✅ ถูก - ส่ง sd โดยตรง (จะถูก cast เป็น block_list* อัตโนมัติ)
status_calc_bl(sd, { SCB_SPEED });
```

> 📖 **อ้างอิง:** `src/map/pc.hpp` - `class map_session_data : public block_list`

## Custom Include Files (*.inc)
**⚠️ NOTE:** ใช้งานโฟลเดอร์ `src/custom/` มีไฟล์ `.inc` สำหรับเพิ่ม custom code โดยไม่ต้องแก้ไข core source files: ก่อนเสมอ

### Script Commands

| ไฟล์ | คำอธิบาย |
|------|----------|
| `script.inc` | **ตัวฟังก์ชัน** ของ custom script commands |
| `script_def.inc` | **ลงทะเบียน** custom script commands |
| `script_constants_custom.inc` | เพิ่ม constants สำหรับใช้ใน NPC scripts |

```cpp
// ตัวอย่าง: เพิ่ม custom script command

// 1. script_def.inc - ลงทะเบียน
BUILDIN_DEF(mycmd,"i"),

// 2. script.inc - ตัวฟังก์ชัน
BUILDIN_FUNC(mycmd)
{
	int val = script_getnum(st, 2);
	script_pushint(st, val * 2);
	return SCRIPT_CMD_SUCCESS;
}
```

### @ Commands

| ไฟล์ | คำอธิบาย |
|------|----------|
| `atcommand_def.inc` | **ลงทะเบียน** custom @commands |
| `atcommand.inc` | **ตัวฟังก์ชัน** ของ custom @commands |

```cpp
// ตัวอย่าง: เพิ่ม custom @command

// 1. atcommand_def.inc - ลงทะเบียน
ACMD_DEF(mycommand),

// 2. atcommand.inc - ตัวฟังก์ชัน
ACMD_FUNC(mycommand)
{
	clif_displaymessage(fd, "It works!");
	return 0;
}
```

### Battle Config (conf settings)

| ไฟล์ | คำอธิบาย |
|------|----------|
| `battle_config_struct.inc` | **ประกาศตัวแปร** ใน battle_config struct |
| `battle_config_init.inc` | **กำหนดค่าเริ่มต้น** และ mapping กับ conf file |

```cpp
// ตัวอย่าง: เพิ่ม custom battle config

// 1. battle_config_struct.inc - ประกาศ
int my_custom_rate;

// 2. battle_config_init.inc - init
{ "my_custom_rate", &battle_config.my_custom_rate, 100, 0, 1000 },

// 3. ใช้ใน conf/battle/*.conf
my_custom_rate: 150
```

### Status & Effects

| ไฟล์ | คำอธิบาย |
|------|----------|
| `status_custom.inc` | เพิ่ม custom SC_* (status change) |
| `efst_custom.inc` | เพิ่ม custom EFST_* (client-side icons) |

```cpp
// status_custom.inc - เพิ่ม status change
SC_MY_BUFF,
SC_MY_DEBUFF,

// efst_custom.inc - เพิ่ม effect icon (ต้องตรงกับ client)
EFST_MY_ICON = 2000,
```

### Map Flags

| ไฟล์ | คำอธิบาย |
|------|----------|
| `mapflag_custom.inc` | เพิ่ม custom MF_* (map flags) |

```cpp
// mapflag_custom.inc - เพิ่ม mapflag
MF_MY_CUSTOM_FLAG,
```

### ⚠️ สำคัญ: Export Constants สำหรับ NPC Scripts

เมื่อเพิ่ม custom constants ในไฟล์ *.inc ใดๆ และต้องการให้ใช้งานได้ใน NPC scripts **ต้อง export ด้วย `export_constant()`**

**ไฟล์:** `src/custom/script_constants_custom.inc`

```cpp
// Export constants ให้ NPC scripts เรียกใช้ได้

// ตัวอย่าง: เมื่อเพิ่ม SC_* ใน status_custom.inc
export_constant(SC_MY_CUSTOM_STATUS);

// ตัวอย่าง: เมื่อเพิ่ม EFST_* ใน efst_custom.inc
export_constant(EFST_MY_CUSTOM_ICON);

// ตัวอย่าง: เมื่อเพิ่ม MF_* ใน mapflag_custom.inc
export_constant(MF_MY_CUSTOM_FLAG);

// ตัวอย่าง: Custom values
export_constant(MY_CUSTOM_CONSTANT);
```

**หมายเหตุ:**
- ใช้ `export_constant()` เพื่อให้ NPC scripts อ้างถึงค่า constants ได้
- ต้อง include ไฟล์ที่เกี่ยวข้องก่อน (เช่น status_custom.inc, efst_custom.inc)
- หลังจากแก้ไขต้อง recompile server

**ตัวอย่างการใช้ใน NPC Script:**
```c
// หลังจาก export_constant แล้ว สามารถใช้ใน NPC ได้เลย
if (checkoption(SC_MY_CUSTOM_STATUS)) {
    mes "คุณมี status effect นี้";
}

// หรือใช้กับ mapflag
prontera mapflag MF_MY_CUSTOM_FLAG
```

### ข้อดีของการใช้ *.inc files

1. **ไม่ต้องแก้ไข core files** - ง่ายต่อการ update rAthena
2. **แยก custom code ชัดเจน** - ง่ายต่อการ backup และ migrate
3. **ลด merge conflicts** - เมื่อ pull updates จาก upstream

## Database Files (YAML)

### ไฟล์สำคัญใน `db/`

| ไฟล์ | คำอธิบาย |
|------|----------|
| `item_db.yml` | ข้อมูลไอเทมทั้งหมด |
| `mob_db.yml` | ข้อมูล monster |
| `skill_db.yml` | ข้อมูล skill |
| `quest_db.yml` | ข้อมูล quest |
| `instance_db.yml` | ข้อมูล instance dungeons |

### รูปแบบ YAML

```yaml
- Id: 501
  AegisName: Red_Potion
  Name: Red Potion
  Type: Healing
  Buy: 50
  Weight: 70
  Script: |
    itemheal rand(45,65),0;
```

## Configuration Files

### ไฟล์ใน `conf/`

| ไฟล์ | คำอธิบาย |
|------|----------|
| `login_athena.conf` | Login server config |
| `char_athena.conf` | Character server config |
| `map_athena.conf` | Map server config |
| `inter_athena.conf` | Database connections |
| `battle/*.conf` | Battle settings |
| `groups.yml` | GM group permissions |

## คำสั่งที่มีประโยชน์

### Build Commands (Windows)

# เปิด rAthena.sln ด้วย Visual Studio แล้ว Build

# หรือใช้ CMake
cmake -B build -G "Visual Studio 18 2026"
cmake --build build --config Release

```batch
# Start all servers
runserver.bat

# Start individual
logserv.bat
charserv.bat
mapserv.bat
```

### ในเกม (GM Commands)

```
@reloadscript     - โหลด NPC script ใหม่
@reloaditemdb     - โหลด item database ใหม่
@reloadmobdb      - โหลด mob database ใหม่
@item <id> <amt>  - สร้างไอเทม
@warp <map> <x> <y> - วาร์ป
@go <city_number> - วาร์ปไปเมือง
```

## เอกสารใน doc/

โฟลเดอร์ `doc/` มีเอกสารสำคัญมากมายที่ควรอ่านก่อนพัฒนา:

### Script & Commands

| ไฟล์ | คำอธิบาย |
|------|----------|
| `script_commands.txt` | **สำคัญมาก!** คำสั่ง script ทั้งหมดพร้อมตัวอย่าง |
| `atcommands.txt` | รายละเอียด @commands และ #commands |
| `permissions.txt` | ระบบ permission สำหรับ GM groups |

### Item System

| ไฟล์ | คำอธิบาย |
|------|----------|
| `item_db.txt` | โครงสร้าง item database |
| `item_bonus.txt` | รายละเอียด item bonus ทั้งหมด (เช่น bStr, bAtk) |
| `item_group.txt` | ระบบ item group |

### Monster System

| ไฟล์ | คำอธิบาย |
|------|----------|
| `mob_db.txt` | โครงสร้าง mob database |
| `mob_db_mode_list.txt` | รายการ mob mode flags (เช่น MD_BOSS, MD_DETECTOR) |
| `mob_avail.txt` | ระบบ sprite replacement |
| `mob_skill_db_powerskill.txt` | power skill ของ mob |
| `mob_item_ratio.txt` | อัตรา drop rate adjustment |

### Skill System

| ไฟล์ | คำอธิบาย |
|------|----------|
| `skill_db.txt` | โครงสร้าง skill database |

### Status System

| ไฟล์ | คำอธิบาย |
|------|----------|
| `status.txt` | รายละเอียด status types |
| `status_change.txt` | รายละเอียด status change effects (SC_*) |

### Map System

| ไฟล์ | คำอธิบาย |
|------|----------|
| `mapflags.txt` | mapflag ทั้งหมด (เช่น pvp, gvg, noteleport) |
| `map_cache.txt` | ระบบ map cache |

### Quest System

| ไฟล์ | คำอธิบาย |
|------|----------|
| `quest_db.txt` | โครงสร้าง quest database |
| `quest_variables.txt` | ตัวแปรของ quest |

### Source Code Documentation

| ไฟล์ | คำอธิบาย |
|------|----------|
| `source_doc.txt` | เอกสารโครงสร้าง source code |
| `packet_struct_notation.md` | รูปแบบ packet structure |
| `packet_interserv.txt` | inter-server packets |

### อื่นๆ

| ไฟล์ | คำอธิบาย |
|------|----------|
| `achievements.md` | ระบบ achievement |
| `effect_list.md` | รายการ effects/visual |
| `woe_time_explanation.txt` | ระบบเวลา WoE |
| `ea_job_system.txt` | ระบบ job classes |

### โฟลเดอร์ย่อย

| โฟลเดอร์ | คำอธิบาย |
|----------|----------|
| `sample/` | ตัวอย่าง NPC scripts หลากหลายประเภท |
| `yaml/` | เอกสารรูปแบบ YAML สำหรับ database files ทั้งหมด |
| `model/` | โมเดลข้อมูล |

## Tips สำหรับการพัฒนา

1. **อ่าน `doc/script_commands.txt`** - เอกสารนี้มีคำสั่ง script ทั้งหมดพร้อมตัวอย่าง
2. **ดูตัวอย่างใน `npc/custom/`** - มี NPC ตัวอย่างหลายตัว
3. **ตรวจสอบ `doc/` ก่อนแก้ไข** - มีเอกสารสำหรับทุกระบบ
4. **Backup ก่อนแก้ไข** - โดยเฉพาะไฟล์ใน `db/` และ `conf/`
5. **ใช้ SQL upgrades** - ตรวจสอบไฟล์ใน `sql-files/upgrades/`
6. **เพิ่ม SRC ใหม่** - ต้องดูใน `src/custom/` และ `conf/` ก่อนเพิ่ม

## Utility Functions (C++)

### format_number_comma - Format ตัวเลขด้วย comma

**⚠️ สำคัญ:** เมื่อต้องแสดงข้อความที่มีจำนวนตัวเลขให้ผู้เล่นเห็น ให้ใช้ฟังก์ชัน `format_number_comma` เพื่อให้อ่านง่าย

**ตำแหน่งไฟล์:**
- Declaration: `src/map/unit.hpp`
- Implementation: `src/map/unit.cpp`

**Signature:**
```cpp
char* format_number_comma(int64 num, char* buf, size_t size);
```

**วิธีใช้งาน:**
```cpp
#include "unit.hpp"

// ตัวอย่าง: แสดงข้อความแจ้งเตือนผู้เล่น
char points_str[32], total_str[32];
char message[256];

format_number_comma(points, points_str, sizeof(points_str));
format_number_comma(total, total_str, sizeof(total_str));

snprintf(message, sizeof(message), "Received %s points (Total: %s points)", points_str, total_str);
clif_messagecolor(sd, color_table[COLOR_LIGHT_GREEN], message, false, SELF);
```

**ผลลัพธ์:**
- `1234567` → `"1,234,567"`
- `-500000` → `"-500,000"`
- `999` → `"999"`

**ใช้งานเมื่อ:**
- แสดง Zeny, EXP, Points หรือจำนวนตัวเลขใดๆ ให้ผู้เล่นเห็น
- สร้างข้อความ announce หรือ notification
- แสดงข้อความใน NPC dialog (ผ่าน script command หรือ C++)

---

## การจัดการ Visual Studio Project Files สำหรับ Custom Code

### ปัญหา
เมื่อเพิ่ม custom source files เข้าไปใน project `map-server.vcxproj` โดยตรง จะทำให้:
- เกิด conflict เมื่อ merge จาก upstream rAthena
- ยากต่อการ maintain ว่าไฟล์ไหนเป็น custom ของเรา
- ไฟล์ `.vcxproj.filters` จะรกและสับสน

### วิธีแก้ไข (แบบ Property Sheet - แนะนำ)

สร้าง **Property Sheet แยก** เก็บ custom files แล้ว import เข้า project หลัก:

**1. สร้างไฟล์ `src/map/map-server-custom.props`:**

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <!-- Custom Header Files -->
    <ClInclude Include="$(MSBuildThisFileDirectory)map_mf_tb.hpp" />
    <ClInclude Include="$(MSBuildThisFileDirectory)custom_feature.hpp" />
    
    <!-- Custom Source Files -->
    <ClCompile Include="$(MSBuildThisFileDirectory)map_mf_tb.cpp" />
    <ClCompile Include="$(MSBuildThisFileDirectory)custom_feature.cpp" />
  </ItemGroup>
  
  <ItemGroup>
    <!-- Filters สำหรับ Solution Explorer -->
    <ClInclude Include="$(MSBuildThisFileDirectory)map_mf_tb.hpp">
      <Filter>Custom Files\Headers</Filter>
    </ClInclude>
    <ClCompile Include="$(MSBuildThisFileDirectory)map_mf_tb.cpp">
      <Filter>Custom Files\Sources</Filter>
    </ClCompile>
  </ItemGroup>
</Project>
```

**2. แก้ไข `map-server.vcxproj` เพิ่มบรรทัด import:**

```xml
<!-- ใส่ต่อท้ายไฟล์ ก่อน </Project> -->
<Import Project="map-server-custom.props" />
```

**ข้อดี:**
- Custom files แยกออกมาชัดเจน
- ไม่ต้องแก้ไข `map-server.vcxproj` หลัก → ลด conflict ตอน merge
- ง่ายต่อการ maintain (รู้ว่าอะไรเป็น custom ของเรา)
- สามารถ version control แยกได้

### วิธีแก้ไข (แบบ Project Reference)

สร้าง **Project แยก** สำหรับ custom code แล้ว link เข้าไป:

**1. สร้างไฟล์ `src/map/map-server-custom.vcxproj`:**

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="12.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <!-- ... config อื่นๆ ... -->
  </ItemGroup>
  
  <ItemGroup>
    <ClInclude Include="map_mf_tb.hpp" />
    <ClCompile Include="map_mf_tb.cpp" />
  </ItemGroup>
  
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <!-- ... -->
</Project>
```

**2. เพิ่ม Reference ใน `map-server.vcxproj`:**

```xml
<ItemGroup>
  <ProjectReference Include="map-server-custom.vcxproj">
    <Project>{GUID-OF-CUSTOM-PROJECT}</Project>
  </ProjectReference>
</ItemGroup>
```

**ข้อควรระวัง:**
- แบบ Project Reference ซับซ้อนกว่า (ต้องจัดการ dependency)
- แนะนำใช้ **แบบ Property Sheet** สำหรับ most use cases

### TODO สำหรับ Branch Master
- [ ] สร้าง `map-server-custom.props`
- [ ] ย้าย custom files (เช่น `map_mf_tb.*`) ไปอยู่ใน props
- [ ] อัปเดต `map-server.vcxproj` ให้ import props แทนการใส่ไฟล์ตรง
- [ ] ทดสอบ build ทั้ง Debug และ Release
- [ ] อัปเดตเอกสารนี้เมื่อเสร็จ

---

## การจัดการ Conflicted Files ใน Git

เมื่อเกิด conflict ในไฟล์ สามารถใช้แนวทางต่อไปนี้:

### แบบ Cowork Agent

ใช้เมื่อต้องการให้ agent อื่นช่วยแก้ไข conflict โดยเฉพาะ:

```
ขอให้ @general ช่วย resolve conflict ในไฟล์ src/map/battle.cpp ตั้งแต่ line 500-600
```

**ขั้นตอน:**
1. ระบุไฟล์และบริเวณที่ conflict
2. ส่งต่อให้ agent อื่นวิเคราะห์
3. ตรวจสอบคำแนะนำและนำมารวมกัน

### แบบ Multi-Agent (Pseudo-Parallel)

ใช้เมื่อต้องการให้หลาย agent ทำงานพร้อมกัน:

```json
{
  "agents": [
    {"name": "agent1", "task": "วิเคราะห์ conflict ในไฟล์ NPC"},
    {"name": "agent2", "task": "วิเคราะห์ conflict ในไฟล์ C++"},
    {"name": "agent3", "task": "วิเคราะห์ conflict ใน database"}
  ]
}
```

**ข้อดี:**
- ประหยัดเวลาเมื่อมีหลาย conflict
- แต่ละ agent เชี่ยวชาญในด้านต่างกัน
- รวมผลลัพธ์แล้วค่อย merge

### แบบ Sub-Agent

ใช้เมื่อต้องการแบ่งงานเป็นขั้นตอน:

```
1. สร้าง sub-agent สำหรับวิเคราะห์ conflict
2. ให้แต่ละ sub-agent รับผิดชอบส่วนต่างๆ
3. รวมผลลัพธ์และแก้ไขด้วยตนเอง
```

**ตัวอย่าง Sub-Agent:**
- **Script Agent**: ดูแล NPC scripts
- **Cpp Agent**: ดูแล C++ source code
- **DB Agent**: ดูแล database YAML files

### เครื่องมือที่ใช้ในการแก้ไข Conflict

| เครื่องมือ | คำอธิบาย |
|-----------|----------|
| `git diff` | ดูความแตกต่างระหว่างเวอร์ชัน |
| `git status` | ดูไฟล์ที่มี conflict |
| `git mergetool` | เปิดเครื่องมือ merge |
| VS Code Merge Editor | แก้ไข conflict ผ่าน UI |

### ขั้นตอนการ Resolve Conflict

1. **ระบุไฟล์ที่ conflict:**
   ```bash
   git status
   ```

2. **เปิดดู conflict:**
   ```bash
   git diff --name-only --diff-filter=U
   ```

3. **แก้ไขแต่ละไฟล์:**
   - เปิดไฟล์ใน editor
   - ดู <<<<<<< ======= >>>>>>>
   - เลือกหรือรวม code ที่ถูกต้อง

4. **ทำเครื่องหมายว่าแก้ไขแล้ว:**
   ```bash
   git add <file>
   ```

5. **Commit merge:**
   ```bash
   git commit -m "Merge branch and resolve conflicts"
   ```

### Best Practices

- **Commit บ่อยๆ** ลดโอกาสเกิด conflict
- **Pull ก่อนทำงาน** ให้เป็นประจำ
- **แก้ไขทีละไฟล์** อย่าแก้หลายไฟล์พร้อมกัน
- **ทดสอบหลัง merge** ให้แน่ใจว่าโค้ดทำงานได้

---

## ระบบ YAML Database

### โครงสร้างไฟล์ YAML

ทุกไฟล์ YAML database มี 3 ส่วนหลัก:

```yaml
Header:
  Type: ITEM_DB      # ชนิด database (ต้องตรงกับ code)
  Version: 3         # เวอร์ชัน schema

Body:
  - Id: 501
    AegisName: Red_Potion
    Name: Red Potion
    Type: Healing
    # ... fields อื่นๆ

Footer:
  Imports:
  - Path: db/re/item_db_usable.yml
  - Path: db/pre-re/item_db_usable.yml
    Mode: Prerenewal
```

### ไดเรกทอรี Database

| Path | คำอธิบาย |
|------|----------|
| `db/` | Database กลาง (shared) + Footer imports |
| `db/re/` | Renewal mode (57 ไฟล์) |
| `db/pre-re/` | Pre-renewal mode (40 ไฟล์) |
| `db/import/` | Override สำหรับแต่ละ server |

### Class Hierarchy (C++)

```
YamlDatabase (abstract base - src/common/database.hpp)
├── TypesafeYamlDatabase<KeyType, DataType>
│   └── TypesafeCachedYamlDatabase<KeyType, DataType>
│       ├── ItemDatabase<t_itemid, item_data>
│       ├── MobDatabase<uint16, mob_db>
│       ├── SkillDatabase<uint32, s_skill_db>
│       └── ... (database อื่นๆ)
```

### วิธีเพิ่ม/แก้ไข YAML Entry

**ขั้นตอนสร้าง custom database entry:**
1. เปิดไฟล์ `db/re/<type>_db.yml` หรือ `db/import/<type>_db.yml`
2. เพิ่ม entry ใหม่ใน Body section
3. ใช้ `@reloaditemdb` / `@reloadmobdb` / `@reloadskilldb` ในเกม

### การ Parse YAML ใน C++ (RapidYAML)

```cpp
// ใน subclass ของ YamlDatabase ต้อง implement:
uint64 parseBodyNode(const ryml::NodeRef& node) override {
    t_itemid id;
    if (!this->asUInt32(node, "Id", id))
        return 0;

    // ตรวจสอบว่ามีอยู่แล้วหรือไม่
    auto item = this->find(id);
    bool exists = item != nullptr;

    if (!exists) {
        item = std::make_shared<item_data>();
        item->nameid = id;
    }

    // Parse fields
    if (this->nodeExists(node, "Name")) {
        std::string name;
        this->asString(node, "Name", name);
        // ...
    }

    this->put(id, item);
    return 1;
}
```

**Helper functions ที่สำคัญ:**

| Function | คำอธิบาย |
|----------|----------|
| `nodeExists(node, "FieldName")` | ตรวจว่า field มีอยู่ |
| `asInt32(node, "Name", &out)` | อ่านค่า int32 |
| `asUInt32(node, "Name", &out)` | อ่านค่า uint32 |
| `asString(node, "Name", out)` | อ่านค่า string |
| `asBool(node, "Name", &out)` | อ่านค่า bool |
| `invalidWarning(node, fmt, ...)` | แจ้ง warning พร้อมบรรทัด |

---

## ระบบ Packet / Clif (Client Interface)

### ไฟล์สำคัญ

| ไฟล์ | คำอธิบาย |
|------|----------|
| `src/map/clif.cpp` | ส่ง/รับ packet ระหว่าง server กับ client |
| `src/map/clif.hpp` | Header สำหรับ clif functions |
| `src/map/packets.hpp` | Packet structure definitions |

### การส่ง Packet

**ขั้นตอนหลัก:**
1. สร้าง buffer ด้วย `WFIFOHEAD(fd, size)`
2. ใส่ packet ID: `WFIFOW(fd, 0) = 0xXXXX`
3. ใส่ข้อมูล: `WFIFOW/WFIFOL/WFIFOB`
4. ส่งผ่าน `clif_send(buf, len, bl, target)`

**Send Target (enum send_target):**

| Target | คำอธิบาย |
|--------|----------|
| `ALL_CLIENT` | ทุก client ที่เชื่อมต่อ |
| `AREA` | ผู้เล่นในพื้นที่รอบๆ |
| `AREA_WOS` | พื้นที่ ยกเว้นตัวเอง |
| `PARTY` | สมาชิก party |
| `GUILD` | สมาชิก guild |
| `SELF` | ตัวเองเท่านั้น |

### Macros สำหรับ Packet

**Position Encoding (3 bytes):**
```cpp
WBUFPOS(buf, offset, x, y, direction)   // เขียนพิกัด
RBUFPOS(buf, offset, &x, &y, &dir)      // อ่านพิกัด
WBUFPOS2(buf, offset, x0, y0, x1, y1, sx, sy)  // Movement vector (6 bytes)
```

**Type Conversion:**

| Macro/Function | คำอธิบาย |
|----------------|----------|
| `client_tick(tick)` | แปลง t_tick → uint32 |
| `client_exp(exp)` | Cap experience ตาม PACKETVER |
| `client_index(idx)` | inventory: server + 2 |
| `server_index(idx)` | inventory: client - 2 |
| `client_nameid(id)` | ใช้ viewid หรือ original id |
| `disguised_bl_id(id)` | คืนค่า negative id สำหรับ disguise |

### Area Broadcasting

- ใช้ `AREA_SIZE` (ปกติ 14-20 cells) เป็นรัศมี
- `map_foreachinallarea()` วนหา block_list ในพื้นที่
- `clif_send_sub()` ตรวจสอบ session ของแต่ละ client

---

## ระบบ Status / Status Change

### โครงสร้าง status_data

```cpp
struct status_data {
    uint32 hp, sp, ap;
    uint32 max_hp, max_sp, max_ap;
    int16 str, agi, vit, int_, dex, luk;        // Base stats
    int16 pow, sta, wis, spl, con, crt;          // 4th job stats
    int32 batk;                                   // Base ATK
    uint16 watk, matk_min, matk_max;
    uint16 speed, amotion, adelay, dmotion;
    int16 hit, flee, cri, flee2, def2, mdef2;
    defType def, mdef;                            // RENEWAL dependent type
    unsigned char def_ele, ele_lv, size, race, class_;
    struct weapon_atk rhw, lhw;                   // อาวุธมือขวา/ซ้าย
};
```

### Status API Functions

```cpp
// เริ่ม status change
int32 status_change_start(src_bl, target_bl, type, rate, val1, val2, val3, val4, duration, flags, delay);

// จบ status change
int32 status_change_end(block_list *bl, enum sc_type type, int32 tid);

// Shorthand wrappers
sc_start(bl, type, rate, val1, tick, flag);        // 1 value
sc_start2(bl, type, rate, val1, val2, tick, flag);  // 2 values
sc_start4(bl, type, rate, v1, v2, v3, v4, tick, flag); // 4 values

// Damage/Heal
int32 status_damage(src, target, dhp, dsp, dap, walkdelay, flag, skill_id);
int32 status_heal(bl, hhp, hsp, hap, flag);
```

**status_heal flag meanings:**

| Flag | คำอธิบาย |
|------|----------|
| `& 1` | Forced healing (ข้ามข้อจำกัด Berserk) |
| `& 2` | แสดง heal effect |
| `& 4` | แสดง HP heal effect แม้ heal 0 |
| `& 16` | Coma damage (HP/SP เหลือ 1) |

### Status Access Functions

```cpp
status_get_status_data(bl)    // ค่า status ที่คำนวณแล้ว
status_get_base_status(bl)    // ค่า base stats (ก่อน modify)
status_get_hp(bl)             // HP ปัจจุบัน
status_get_str/agi/vit/int/dex/luk(bl)  // แต่ละ stat
```

### ASPD Constants

| Constant | ค่า | คำอธิบาย |
|----------|-----|----------|
| `MIN_ASPD` | 8000 | Delay สูงสุด (ป้องกัน ASPD ต่ำเกิน) |
| `MAX_ASPD_NOPC` | 100 | Delay ต่ำสุดสำหรับ non-player |
| `AMOTION_ZERO_ASPD` | 2000 | Base amotion = 0 ASPD |
| `AMOTION_INTERVAL` | 10 | ทุก 1 ASPD ลด amotion 10ms |

---

## ระบบ Battle Calculation

### โครงสร้าง Damage

```cpp
struct Damage {
#ifdef RENEWAL
    int64 statusAtk, statusAtk2;     // Status-based ATK
    int64 weaponAtk, weaponAtk2;     // Weapon ATK
    int64 equipAtk, equipAtk2;      // Equipment ATK
    int64 masteryAtk, masteryAtk2;  // Mastery bonus
    int64 percentAtk, percentAtk2;  // % bonuses
#else
    int64 basedamage;               // PRE-RE: Base damage
#endif
    int64 damage, damage2;          // มือขวา/ซ้าย final damage
    enum e_damage_type type;        // DMG_NORMAL, DMG_SPLASH, etc.
    int16 div_;                     // จำนวน hits
    int32 amotion, dmotion;         // Animation delays
    int32 blewcount;                // Knockback
    int32 flag;                     // e_battle_flag bits
    bool isspdamage;                // Blue damage numbers (SP damage)
};
```

### Damage Calculation Pipeline

```
1. battle_calc_attack(attack_type, src, target, skill_id, skill_lv, flag)
   ↓ dispatch ตาม attack_type
2. battle_calc_weapon_attack() / battle_calc_magic_attack() / battle_calc_misc_attack()
   ↓ คำนวณ damage ตาม formula
3. battle_calc_defense_reduction()
   ↓ ลด damage ตาม DEF/MDEF
4. battle_calc_damage(src, bl, &wd, damage, skill_id, skill_lv)
   ↓ ปรับ GVG/BG/PK reductions
5. battle_damage() → status_damage()
   ↓ apply damage จริง
6. clif_damage() → broadcast ไปยังผู้เล่นในพื้นที่
```

### Damage ATK Macros

```cpp
// คูณ damage ด้วย rate (มือขวา + ซ้าย)
ATK_RATE(damage, damage2, rate_percent)

// คูณ damage แยก rate มือขวา/ซ้าย
ATK_RATE2(damage, damage2, rate_right, rate_left)

// เพิ่ม damage
ATK_ADD(damage, damage2, value)
ATK_ADD2(damage, damage2, value_right, value_left)
```

### Battle Check Target (BCT_*)

| Flag | คำอธิบาย |
|------|----------|
| `BCT_SELF` | ตัวเอง |
| `BCT_ENEMY` | ศัตรู |
| `BCT_PARTY` | สมาชิก party |
| `BCT_GUILD` | สมาชิก guild |
| `BCT_ALLY` | พันธมิตร |
| `BCT_WOS` | ไม่รวมตัวเอง |
| `BCT_ALL` | ทุกเป้าหมาย |

---

## ระบบ Skill

### โครงสร้าง s_skill_condition (Requirements)

```cpp
struct s_skill_condition {
    int32 hp, hp_rate;                         // ค่า HP ที่ใช้
    int32 sp, sp_rate;                         // ค่า SP ที่ใช้
    int32 ap, ap_rate;                         // ค่า AP (4th job)
    int32 zeny;                                // ค่า Zeny
    int32 weapon;                              // Bitmask อาวุธที่ต้องใช้
    int32 ammo, ammo_qty;                      // ชนิด/จำนวนกระสุน
    int32 state;                               // สถานะที่ต้องการ
    int32 spiritball;                          // จำนวน Spirit Sphere
    t_itemid itemid[MAX_SKILL_ITEM_REQUIRE];   // ไอเทมที่ต้องใช้
    int32 amount[MAX_SKILL_ITEM_REQUIRE];      // จำนวนไอเทม
    std::vector<t_itemid> eqItem;              // อุปกรณ์ที่ต้องสวม
    std::vector<sc_type> status;               // Status ที่ต้องมี
};
```

### โครงสร้าง Skill Unit Group

```cpp
struct s_skill_unit_group {
    int32 src_id;                   // ID ผู้ใช้ skill
    int32 party_id, guild_id;      // สังกัด
    int32 map;                     // แผนที่
    int32 target_flag;             // BCT_* เป้าหมาย
    t_tick tick, limit;            // เวลาเริ่ม/หมดอายุ
    int32 interval;                // ช่วง timer
    uint16 skill_id, skill_lv;    // skill ที่ใช้
    int32 val1, val2, val3;       // ค่าปรับแต่ง
    int32 unit_id;                // Client effect ID
};

struct skill_unit : public block_list {
    std::shared_ptr<s_skill_unit_group> group;
    t_tick limit;
    int32 val1, val2;
    int16 range;
    bool alive, hidden;
};
```

### Skill Unit Flags (e_skill_unit_flag)

| Flag | คำอธิบาย |
|------|----------|
| `UF_NOENEMY` | ไม่มีผลกับศัตรู |
| `UF_NOREITERATION` | ห้ามวางซ้อน |
| `UF_NOFOOTSET` | ห้ามวางใต้ตัว |
| `UF_PATHCHECK` | ตรวจสอบเส้นทาง |
| `UF_DANCE` / `UF_SONG` / `UF_ENSEMBLE` | สกิลดนตรี |
| `UF_NOKNOCKBACK` | ห้าม knockback |
| `UF_HIDDENTRAP` | กับดักซ่อน |

---

## Core Types จาก map.hpp

### block_list (โครงสร้างพื้นฐาน)

```cpp
struct block_list {
    struct block_list *next, *prev;  // Linked list (map grid)
    int32 id;                        // Unique ID
    int16 m, x, y;                   // Map index, พิกัด X, Y
    enum bl_type type;               // ชนิดของ entity
};
```

### Block List Types (bl_type)

| Type | คำอธิบาย |
|------|----------|
| `BL_PC` | Player character |
| `BL_MOB` | Monster |
| `BL_PET` | Pet |
| `BL_HOM` | Homunculus |
| `BL_MER` | Mercenary |
| `BL_ELEM` | Elemental |
| `BL_NPC` | NPC |
| `BL_SKILL` | Skill unit (พื้นที่สกิล) |
| `BL_ITEM` | ไอเทมบนพื้น |

### Element Types (e_element)

| Enum | คำอธิบาย |
|------|----------|
| `ELE_NEUTRAL` | ธาตุปกติ |
| `ELE_WATER` | น้ำ |
| `ELE_EARTH` | ดิน |
| `ELE_FIRE` | ไฟ |
| `ELE_WIND` | ลม |
| `ELE_POISON` | พิษ |
| `ELE_HOLY` | ศักดิ์สิทธิ์ |
| `ELE_DARK` | มืด |
| `ELE_GHOST` | วิญญาณ |
| `ELE_UNDEAD` | อมตะ |

### Job ID System (e_mapid) - Bitmask

```cpp
#define JOBL_2_1    0x100      // 2nd job tier 1
#define JOBL_2_2    0x200      // 2nd job tier 2
#define JOBL_THIRD  0x1000     // 3rd job
#define JOBL_FOURTH 0x10000    // 4th job
#define JOBL_UPPER  0x100000   // Transcendent
#define JOBL_BABY   0x200000   // Baby class

// Masks
#define MAPID_FIRSTMASK   0xff      // 1st class
#define MAPID_SECONDMASK  0xfff     // ถึง 2nd job
#define MAPID_THIRDMASK   0xffff    // ถึง 3rd job
#define MAPID_FOURTHMASK  0xfffff   // ถึง 4th job
```

### Damage Type (e_damage_type)

| Type | คำอธิบาย |
|------|----------|
| `DMG_NORMAL` | Damage ปกติ |
| `DMG_ENDURE` | Endure effect |
| `DMG_SPLASH` | Area damage |
| `DMG_MULTI_HIT` | หลาย hit |
| `DMG_MULTI_HIT_ENDURE` | หลาย hit + endure |

### Auto Trigger Flags (item bonus)

| Flag | คำอธิบาย |
|------|----------|
| `ATF_SELF` | Trigger บนตัวเอง |
| `ATF_TARGET` | Trigger บนเป้าหมาย |
| `ATF_SHORT` | ระยะใกล้ |
| `ATF_LONG` | ระยะไกล |
| `ATF_WEAPON` | โจมตีกายภาพ |
| `ATF_MAGIC` | โจมตีเวทย์ |
| `ATF_MISC` | โจมตีอื่นๆ |

### Refine Constants

```cpp
// RENEWAL
#define MAX_REFINE 20

// PRE-RENEWAL
#define MAX_REFINE 10
```

| Refine Type | คำอธิบาย |
|-------------|----------|
| `REFINE_TYPE_ARMOR` | เกราะ |
| `REFINE_TYPE_WEAPON` | อาวุธ |
| `REFINE_TYPE_SHADOW_ARMOR` | Shadow เกราะ |
| `REFINE_TYPE_SHADOW_WEAPON` | Shadow อาวุธ |

---

## ระบบ Custom Skill Override

### ไฟล์สำหรับ Custom Skill

| ไฟล์ | คำอธิบาย |
|------|----------|
| `src/map/skills/custom/skill_factory_custom.cpp` | Implementation ของ custom skills |
| `src/map/skills/custom/skill_factory_custom.hpp` | Header สำหรับ custom skills |

**ตัวอย่าง Override Skill:**
```cpp
// skill_factory_custom.cpp
#include "skill_factory_custom.hpp"
// Override SM_BASH ด้วย custom implementation
```

> **หมายเหตุ:** ปัจจุบันไฟล์เหล่านี้เป็น template ว่างเปล่า (code อยู่ใน `#if 0`)

---

## NPC Script เพิ่มเติม: ระบบ Custom NPC

### โฟลเดอร์ npc/custom/ (68 ไฟล์)

| โฟลเดอร์ | จำนวน | ตัวอย่าง |
|----------|--------|----------|
| `npc/custom/` (root) | 13 | warper, healer, jobmaster, card_seller |
| `npc/custom/battleground/` | 2 | bg_emp, bg_pvp |
| `npc/custom/etc/` | 15 | autopot, bank, mvp_arena, stock_market |
| `npc/custom/events/` | 5+4 | devil_square, disguise, holiday events |
| `npc/custom/quests/` | 15+5 | hunting_missions, quest_shop, THQ |

### การเปิดใช้ Custom NPC

1. แก้ไข `npc/scripts_custom.conf`
2. Uncomment บรรทัดที่ต้องการ: `npc: npc/custom/warper.txt`
3. ใช้ `@reloadscript` ในเกม

> **หมายเหตุ:** ค่า default ทุก script จะ comment ไว้ (ปิดทั้งหมด)

---

## Configuration Import System

### วิธีการ Override Config

ใช้ไฟล์ใน `conf/import/` เพื่อ override ค่า default โดยไม่ต้องแก้ไขไฟล์หลัก:

| ไฟล์ Import | Override สำหรับ |
|-------------|-----------------|
| `conf/import/battle_conf.txt` | Battle settings ทั้งหมด |
| `conf/import/char_conf.txt` | Character server |
| `conf/import/map_conf.txt` | Map server |
| `conf/import/login_conf.txt` | Login server |
| `conf/import/inter_conf.txt` | Inter-server communication |
| `conf/import/packet_conf.txt` | Packet version |
| `conf/import/groups.yml` | GM groups & permissions |

**ข้อดี:** ไม่ต้องแก้ไขไฟล์ config หลัก → ลด conflict ตอน merge upstream

---

## Custom Message Configuration (ข้อความ Custom)

### ⚠️ สำคัญ: ห้ามแก้ไข `map_msg.conf` โดยตรง!

เมื่อต้องการเพิ่มข้อความ (message) ใหม่สำหรับ custom features ให้เพิ่มใน **`conf/msg_conf/import/Custom_msg.conf`** เท่านั้น

**ห้ามเพิ่มใน `conf/msg_conf/map_msg.conf`** เพราะจะเกิด conflict เมื่อ merge จาก upstream rAthena

### หมายเลขข้อความ (Message ID)

- **เริ่มต้นจากหมายเลข 3000** เป็นต้นไป
- หมายเลข 0‑2999 สงวนไว้สำหรับ rAthena core

### รูปแบบการเพิ่มข้อความ

```conf
// conf/msg_conf/Custom_msg.conf
3000: ข้อความ custom แรก
3001: ข้อความ custom ที่สอง
3002: Your custom message here
```

### วิธีใช้ใน C++ Source Code

```cpp
// อ่านข้อความจาก message ID
clif_displaymessage(fd, msg_txt(sd, 3000));  // แสดง "ข้อความ custom แรก"
```

### ข้อดี
- ไม่ต้องแก้ไข `map_msg.conf` หลัก → ลด conflict ตอน merge
- แยก custom messages ออกมาชัดเจน
- ง่ายต่อการ backup และ migrate
