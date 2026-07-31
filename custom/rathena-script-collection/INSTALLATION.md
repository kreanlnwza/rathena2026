# rathena-script-collection

ติดตั้งจาก `https://github.com/hayakawakaki/rathena-script-collection/tree/master`

สถานะ: เก็บ collection ใน repository แล้ว แต่ยังไม่เปิดโหลดอัตโนมัติ

เหตุผล: collection มีทั้ง NPC scripts, database files และ source patches ที่อ้างอิงโครงสร้าง/engine รุ่นเก่า การเปิดทั้งหมดพร้อมกันอาจทำให้ parser หรือ build ล้มเหลว จึงต้องเลือกติดตั้งเป็นราย episode หลังตรวจ compatibility กับ branch นี้

## วิธีใช้งาน

1. อ่าน `README.md` และไฟล์ใน episode ที่ต้องการ
2. โหลด `Function.c` ก่อน script อื่นตามคำแนะนำ upstream
3. ย้าย/รวมไฟล์ DB เข้า `db/import/` ตาม schema ของ branch นี้
4. เพิ่ม script เข้า config ที่เกี่ยวข้องหลังตรวจ syntax และ dependency
5. ใช้ไฟล์ `.patch` เฉพาะเมื่อพอร์ต source changes และตรวจสอบกับ source ปัจจุบันแล้ว

ห้ามถือว่าไฟล์ในโฟลเดอร์นี้ถูกเปิดใช้งานจนกว่าจะเพิ่ม import/config อย่างชัดเจน
