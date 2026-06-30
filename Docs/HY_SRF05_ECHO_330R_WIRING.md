# HY-SRF05_ECHO_330R_WIRING.md

# Hướng dẫn cắm HY-SRF05 với ESP32-CAM bằng mạch chia áp 330Ω

> **Tên tài liệu:** HY-SRF05 Echo 330R Voltage Divider Wiring  
> **Phiên bản:** v1.0.0  
> **Ngày tạo:** 2026-06-30  
> **Sửa đổi gần nhất:** 2026-06-30  
> **Mục đích:** Hướng dẫn riêng cách cắm cảm biến siêu âm HY-SRF05 với ESP32-CAM, trong đó chân `Echo` được hạ áp bằng nhiều điện trở `330Ω` để an toàn cho GPIO của ESP32-CAM.

---

## 1. Mục tiêu của sơ đồ này

HY-SRF05 dùng nguồn `5V`, nên chân `Echo` có thể trả tín hiệu mức `5V`.

ESP32-CAM dùng GPIO mức `3.3V`, vì vậy không được cắm `Echo` trực tiếp vào GPIO của ESP32-CAM.

Sơ đồ này dùng nhiều điện trở `330Ω` để tạo mạch chia áp, giúp tín hiệu từ `Echo` hạ xuống khoảng `3.125V` trước khi vào `GPIO12`.

---

## 2. Linh kiện cần dùng

| Linh kiện | Số lượng | Ghi chú |
|---|---:|---|
| HY-SRF05 | 1 | Cảm biến siêu âm |
| ESP32-CAM | 1 | Board điều khiển chính |
| Điện trở `330Ω` | 8 | Dùng để tạo mạch chia áp cho chân `Echo` |
| Dây cắm | Tùy mạch | Nên dùng breadboard để cắm dễ hơn |
| Breadboard | 1 | Khuyến nghị dùng để nối điện trở gọn và chắc |

---

## 3. Nguyên tắc chia áp

Dùng:

```text
R1 = 3 con 330Ω nối tiếp = 990Ω
R2 = 5 con 330Ω nối tiếp = 1650Ω
```

Điện áp sau chia áp:

```text
Vout = 5V × R2 / (R1 + R2)

Vout = 5V × 1650 / (990 + 1650)
Vout ≈ 3.125V
```

`Vout` chính là điểm `ECHO_SAFE`, điểm này sẽ nối vào `GPIO12` của ESP32-CAM.

---

## 4. Bảng cắm HY-SRF05 với ESP32-CAM

| HY-SRF05 | Cắm với |
|---|---|
| `VCC` | `5V` ESP32-CAM |
| `GND` | `GND` ESP32-CAM |
| `Trig` | `GPIO13` ESP32-CAM |
| `Echo` | Qua chuỗi **3 điện trở 330Ω nối tiếp**, rồi tới điểm `ECHO_SAFE` |
| `OUT` | Không cắm |

---

## 5. Bảng cắm điểm ECHO_SAFE

`ECHO_SAFE` là điểm tín hiệu sau khi đã được hạ áp.

| Điểm `ECHO_SAFE` | Cắm với |
|---|---|
| Nhánh 1 | `GPIO12` ESP32-CAM |
| Nhánh 2 | Qua chuỗi **5 điện trở 330Ω nối tiếp** xuống `GND` |

---

## 6. Bảng cắm từng điện trở

### 6.1 Nhánh R1: từ Echo đến ECHO_SAFE

Nhánh này dùng **3 con điện trở 330Ω nối tiếp**.

| Linh kiện | Đầu 1 | Đầu 2 |
|---|---|---|
| Điện trở 330Ω số 1 | `Echo` HY-SRF05 | Điện trở 330Ω số 2 |
| Điện trở 330Ω số 2 | Điện trở 330Ω số 1 | Điện trở 330Ω số 3 |
| Điện trở 330Ω số 3 | Điện trở 330Ω số 2 | Điểm `ECHO_SAFE` |

### 6.2 Nhánh R2: từ ECHO_SAFE xuống GND

Nhánh này dùng **5 con điện trở 330Ω nối tiếp**.

| Linh kiện | Đầu 1 | Đầu 2 |
|---|---|---|
| Điện trở 330Ω số 4 | Điểm `ECHO_SAFE` | Điện trở 330Ω số 5 |
| Điện trở 330Ω số 5 | Điện trở 330Ω số 4 | Điện trở 330Ω số 6 |
| Điện trở 330Ω số 6 | Điện trở 330Ω số 5 | Điện trở 330Ω số 7 |
| Điện trở 330Ω số 7 | Điện trở 330Ω số 6 | Điện trở 330Ω số 8 |
| Điện trở 330Ω số 8 | Điện trở 330Ω số 7 | `GND` ESP32-CAM |

### 6.3 Dây tín hiệu vào ESP32-CAM

| Dây / điểm nối | Cắm với |
|---|---|
| Điểm `ECHO_SAFE` | `GPIO12` ESP32-CAM |

---

## 7. Sơ đồ chữ đầy đủ

```text
HY-SRF05 Echo
     |
   330Ω số 1
     |
   330Ω số 2
     |
   330Ω số 3
     |
     +-----------------> GPIO12 ESP32-CAM
     |
   330Ω số 4
     |
   330Ω số 5
     |
   330Ω số 6
     |
   330Ω số 7
     |
   330Ω số 8
     |
    GND ESP32-CAM
```

---

## 8. Sơ đồ nhớ nhanh

```text
Echo -> 330Ω -> 330Ω -> 330Ω -> ECHO_SAFE -> GPIO12
                                      |
                                    330Ω
                                      |
                                    330Ω
                                      |
                                    330Ω
                                      |
                                    330Ω
                                      |
                                    330Ω
                                      |
                                     GND
```

---

## 9. Bảng tổng hợp ngắn nhất

| Thành phần | Cắm với |
|---|---|
| HY-SRF05 `VCC` | ESP32-CAM `5V` |
| HY-SRF05 `GND` | ESP32-CAM `GND` |
| HY-SRF05 `Trig` | ESP32-CAM `GPIO13` |
| HY-SRF05 `Echo` | 3 con `330Ω` nối tiếp rồi tới `ECHO_SAFE` |
| `ECHO_SAFE` | ESP32-CAM `GPIO12` |
| `ECHO_SAFE` | 5 con `330Ω` nối tiếp rồi xuống `GND` |
| HY-SRF05 `OUT` | Không cắm |

---

## 10. Khai báo chân trong code

```cpp
#define PIN_ULTRASONIC_TRIG 13
#define PIN_ULTRASONIC_ECHO 12
```

---

## 11. Checklist trước khi bật nguồn

```text
[ ] HY-SRF05 VCC đã cắm vào 5V.
[ ] HY-SRF05 GND đã cắm vào GND chung.
[ ] HY-SRF05 Trig đã cắm vào GPIO13.
[ ] HY-SRF05 OUT không cắm.
[ ] Echo không cắm trực tiếp vào GPIO12.
[ ] Từ Echo đến ECHO_SAFE có đúng 3 con điện trở 330Ω nối tiếp.
[ ] Từ ECHO_SAFE xuống GND có đúng 5 con điện trở 330Ω nối tiếp.
[ ] GPIO12 chỉ nối vào điểm ECHO_SAFE.
[ ] Không nối GPIO12 vào phía Echo trước 3 điện trở.
[ ] Không để dây 5V chạm vào GPIO12.
[ ] Tất cả GND của HY-SRF05 và ESP32-CAM đã nối chung.
```

---

## 12. Lỗi cắm nguy hiểm cần tránh

### 12.1 Không cắm Echo trực tiếp

Không cắm:

```text
HY-SRF05 Echo -> GPIO12 ESP32-CAM
```

Vì `Echo` có thể là tín hiệu `5V`, không an toàn cho GPIO của ESP32-CAM.

### 12.2 Không cắm GPIO12 sai vị trí

Không cắm `GPIO12` ở đoạn này:

```text
Echo -> GPIO12 -> 330Ω -> 330Ω -> ...
```

Cách này sai vì `GPIO12` vẫn nhận trực tiếp tín hiệu từ `Echo`.

Phải cắm đúng:

```text
Echo -> 330Ω -> 330Ω -> 330Ω -> ECHO_SAFE -> GPIO12
```

### 12.3 Không bỏ nhánh xuống GND

Nếu chỉ cắm:

```text
Echo -> 330Ω -> 330Ω -> 330Ω -> GPIO12
```

thì đây không phải mạch chia áp đúng. Phải có nhánh từ `ECHO_SAFE` xuống `GND` bằng 5 con điện trở `330Ω`.

---

## 13. Kết luận

Cách cắm dùng **8 con điện trở 330Ω** là phương án phù hợp khi không có điện trở `2.2kΩ`.

Cách chia:

```text
3 con 330Ω từ Echo đến ECHO_SAFE
5 con 330Ω từ ECHO_SAFE xuống GND
GPIO12 nối vào ECHO_SAFE
```

Điện áp vào `GPIO12` khoảng `3.125V`, phù hợp hơn cho ESP32-CAM so với việc cắm `Echo` trực tiếp.
