# Kịch bản thuyết trình tuyến tính cho 4 thành viên

**Dự án:** IoT Based Anti-Theft System - Kitchen Security System  
**Nhóm:** IoT102 - Group 6  
**Nguồn đối chiếu:** Canva 20 slide, `SRS.md`, `DEMO_SCENARIOS_PROPOSAL_v0.1.0.md` và `FLOWCHART_REVIEW_2026-07-12.md`  
**Thời lượng gợi ý:** 16-20 phút, khoảng 4-5 phút mỗi thành viên.

## Cách sử dụng

- Thuyết trình **theo đúng thứ tự tuyến tính**: Long -> Danh -> Vương -> Anh. Mỗi người nói hết phần của mình rồi mới bàn giao.
- Phần trong dấu ngoặc vuông là hành động ngắn trên slide hoặc với mô hình, không cần đọc thành lời.
- Không nói rằng hệ thống "chắc chắn có trộm". Hệ thống chỉ phát hiện **dấu hiệu đột nhập/can thiệp**.
- Với DS-03, cần nói chính xác: email chỉ được Apps Script gửi sau khi đã có DS-04 và thiết bị mất heartbeat quá thời hạn; đây không phải email cho mọi cảnh báo thông thường.

---

# Thành viên 1 - Ngô Gia Long

**Phụ trách:** Slide 1-5  
**Mục tiêu:** Mở đầu, nêu vấn đề, giải pháp và phạm vi dự án.

## Slide 1 - Giới thiệu

> Kính chào thầy/cô và các bạn. Nhóm 6 chúng em xin trình bày dự án **IoT Based Anti-Theft System - Kitchen Security System**. Dự án hướng đến việc giám sát an ninh khu vực bếp bằng các cảm biến, camera và nền tảng IoT. Nhóm gồm bốn thành viên là Ngô Gia Long, Võ Trần Công Danh, Nguyễn An Vương và Nguyễn Nhật Anh.

> Trong phần trình bày này, chúng em sẽ đi từ vấn đề thực tế, phần cứng, nguyên lý hoạt động, cho đến các kịch bản demo đã kiểm thử.

## Slide 2 - Hạn chế của phương pháp truyền thống

> Trước hết là bối cảnh của bài toán. Các biện pháp truyền thống như khóa cơ hoặc chuông báo đơn giản có thể tạo cảnh báo tại chỗ, nhưng còn nhiều hạn chế. Chủ nhà không thể theo dõi từ xa, không nhận được thông báo tức thời và cũng thiếu bằng chứng hình ảnh khi có sự kiện xảy ra.

> Vì vậy, nếu có dấu hiệu bất thường khi không có mặt ở nhà, người dùng thường chỉ biết sau khi sự việc đã xảy ra. Đây là lý do nhóm chọn xây dựng một hệ thống có khả năng phát hiện, thông báo và lưu lại hình ảnh.

## Slide 3 - Yêu cầu của hệ thống an ninh hiện đại

> Từ các hạn chế đó, hệ thống cần đáp ứng ba nhóm yêu cầu. Thứ nhất là giám sát liên tục và phát hiện đột nhập có độ tin cậy tốt hơn bằng cách kết hợp nhiều cảm biến. Thứ hai là thông báo nhanh cho người dùng, phát hiện hành vi che hoặc can thiệp thiết bị, đồng thời hỗ trợ tình huống khẩn cấp. Thứ ba là kết nối IoT để người dùng có thể theo dõi và điều khiển từ xa.

> Giải pháp của nhóm không chỉ là một cảm biến phát hiện đơn lẻ, mà là một luồng xử lý có cảnh báo cục bộ, dashboard, Telegram, camera và cơ chế theo dõi sức khỏe thiết bị.

## Slide 4 - Tổng quan hệ thống

> Hệ thống lấy ESP32-S3 WROOM làm bộ điều khiển trung tâm và dùng camera OV3660 để chụp ảnh. PIR, cảm biến siêu âm và LDR cung cấp dữ liệu về chuyển động, khoảng cách và trạng thái ánh sáng. DS1307 RTC cung cấp thời gian cho lịch tự bật hoặc tắt chế độ chống trộm.

> Về dịch vụ, Arduino Cloud là nơi theo dõi trạng thái và điều khiển từ xa. Telegram gửi thông báo tức thời kèm ảnh khi cần. Google Apps Script nhận heartbeat để theo dõi thiết bị sau sự kiện phá hoại, đồng thời xử lý email xác nhận và escalation mô phỏng. Nhờ đó hệ thống có cả phản hồi tại chỗ lẫn phản hồi từ xa.

## Slide 5 - Nội dung trình bày

> Nội dung tiếp theo gồm ba phần chính. Đầu tiên là phần cứng và chức năng từng thiết bị. Sau đó chúng em trình bày sơ đồ mạch, sơ đồ khối và flowchart để làm rõ cách dữ liệu đi qua hệ thống. Cuối cùng là sáu kịch bản demo, kết luận và hướng phát triển.

> Sau đây, xin mời bạn Võ Trần Công Danh trình bày chi tiết phần phần cứng và chức năng của các thiết bị.

---

# Thành viên 2 - Võ Trần Công Danh

**Phụ trách:** Slide 6-10  
**Mục tiêu:** Giải thích phần cứng và vai trò của từng nhóm cảm biến/thiết bị đầu ra.

## Slide 6 - Các phần cứng chính

> Phần cứng chính của hệ thống gồm ESP32-S3 WROOM, camera OV3660, cảm biến chuyển động PIR, cảm biến siêu âm HY-SRF05, cảm biến ánh sáng LDR, RTC DS1307, buzzer và các LED chỉ thị.

> Nhóm chọn các phần tử này vì mỗi phần tử giải quyết một phần của bài toán: ESP32-S3 xử lý và kết nối mạng; các cảm biến cung cấp dấu hiệu vật lý; camera tạo bằng chứng hình ảnh; còn buzzer và LED tạo phản hồi ngay tại khu vực bếp.

## Slide 7 - ESP32-S3 WROOM

> ESP32-S3 là bộ não của hệ thống. Board đọc dữ liệu cảm biến, quyết định trạng thái cảnh báo, điều khiển LED và buzzer, kích hoạt camera, đồng thời đồng bộ trạng thái với Arduino Cloud.

> Khi có sự kiện, ESP32-S3 cũng gửi thông báo Telegram. Riêng trong luồng chống phá hoại, board gửi heartbeat định kỳ cho Google Apps Script. Nhờ vậy, nếu thiết bị đã phát hiện bị can thiệp rồi sau đó mất liên lạc, dịch vụ phía máy chủ vẫn có thể nhận ra tình trạng này.

## Slide 8 - PIR và cảm biến siêu âm

> Cặp cảm biến PIR và HY-SRF05 được dùng để phát hiện dấu hiệu đột nhập. PIR nhận biết chuyển động hoặc thân nhiệt người trong vùng quan sát. Cảm biến siêu âm đo khoảng cách để xác nhận có vật tiến gần khu vực cảm biến.

> Trong logic hiện tại, cảnh báo đột nhập không chỉ dựa vào điểm số. Hệ thống cần đang được bật bảo vệ, đồng thời PIR có phát hiện chuyển động và vật ở gần trong phạm vi hợp lý liên tục khoảng hai giây. Cách kết hợp này giúp giảm báo động giả so với chỉ dùng một cảm biến.

## Slide 9 - LDR và RTC DS1307

> LDR có hai vai trò. Vai trò thứ nhất là cung cấp bối cảnh ánh sáng cho hệ thống. Vai trò thứ hai quan trọng hơn là hỗ trợ phát hiện phá hoại: khi cảm biến ánh sáng bị che và đồng thời có vật áp sát cảm biến siêu âm trong phạm vi không quá 15 centimet liên tục ba giây, hệ thống nhận đây là dấu hiệu can thiệp thiết bị.

> RTC DS1307 giữ thời gian để chạy lịch tự động. Khi lịch được bật và thời gian hợp lệ, hệ thống có thể tự bật hoặc tự tắt chế độ chống trộm. Tuy nhiên, việc tắt chống trộm theo lịch không làm vô hiệu hóa chức năng SOS và chống phá hoại.

## Slide 10 - Camera, LED và buzzer

> Camera OV3660 chụp ảnh tự động khi có cảnh báo đột nhập hoặc phá hoại, và cũng hỗ trợ chụp thủ công từ dashboard Arduino Cloud. Ảnh sau đó được gửi qua Telegram để người dùng kiểm tra nhanh tình hình khu vực bếp.

> LED và buzzer là phản hồi cục bộ. Khi trạng thái bình thường, LED xanh thể hiện hệ thống đang hoạt động. Khi có cảnh báo, LED đỏ và buzzer được kích hoạt để cảnh báo trực tiếp. Riêng trạng thái SOS sẽ duy trì cảnh báo cho đến khi Parent/Admin chủ động reset.

> Sau khi đã giới thiệu phần cứng, xin mời bạn Nguyễn An Vương trình bày phần kết nối cloud, sơ đồ thiết kế và luồng hoạt động của hệ thống.

---

# Thành viên 3 - Nguyễn An Vương

**Phụ trách:** Slide 11-15  
**Mục tiêu:** Mô tả các dịch vụ cloud, kiến trúc và luồng xử lý đúng với firmware.

## Slide 11 - Các dịch vụ cloud

> Hệ thống sử dụng ba dịch vụ chính. Arduino Cloud hiển thị dashboard để theo dõi trạng thái như cảnh báo hiện tại, khoảng cách, giá trị LDR, trạng thái ảnh và nút điều khiển. Telegram là kênh cảnh báo nhanh, gửi nội dung tiếng Việt và ảnh camera khi có event phù hợp.

> Google Apps Script đảm nhiệm phần heartbeat và escalation. ESP32 gửi heartbeat mỗi 30 giây. Sau khi đã có cảnh báo phá hoại, nếu Apps Script không nhận heartbeat mới trong hơn 60 giây thì hệ thống được nâng thành tình trạng mất liên lạc nghiêm trọng. Bộ trigger của Apps Script kiểm tra mỗi một phút, nên email có thể đến sau khoảng 60 đến 120 giây. Email yêu cầu Parent/Admin xác nhận trước khi gửi báo cáo tới contact mô phỏng.

## Slide 12 - Sơ đồ mạch

[Chỉ vào các nhóm kết nối trên sơ đồ, không cần đọc từng chân nếu thầy/cô không hỏi.]

> Đây là sơ đồ mạch của hệ thống. ESP32-S3 được đặt ở trung tâm, kết nối với nhóm đầu vào gồm PIR, HY-SRF05, LDR và RTC; đồng thời kết nối với nhóm đầu ra gồm camera, LED và buzzer.

> Khi lắp mạch, các cảm biến và module dùng chung nguồn, chung mass, còn các chân tín hiệu được kết nối theo sơ đồ. Vì có camera và kết nối WiFi, nhóm đặc biệt chú ý nguồn ổn định để tránh hiện tượng reset hoặc ảnh chụp không thành công.

## Slide 13 - Sơ đồ khối

> Sơ đồ khối thể hiện hướng đi của dữ liệu. Dữ liệu từ PIR, siêu âm, LDR và RTC đi vào ESP32-S3. Board xử lý theo các điều kiện bảo mật, sau đó điều khiển các đầu ra cục bộ như LED, buzzer và camera.

> Song song với đó, trạng thái được đẩy lên Arduino Cloud. Nếu là event cảnh báo, Telegram nhận thông báo. Nếu là luồng phá hoại rồi mất liên lạc, Google Apps Script theo dõi heartbeat và gửi email xác nhận. Vì vậy kiến trúc có các lớp: phát hiện tại thiết bị, cảnh báo tức thời, giám sát từ xa và escalation có xác nhận.

## Slide 14 - Flowchart

> Về flowchart, hệ thống bắt đầu bằng khởi tạo phần cứng, WiFi, Arduino Cloud và thời gian RTC. Sau đó hệ thống kiểm tra lịch để xác định có bật bảo vệ chống trộm hay không, rồi đọc các cảm biến liên tục.

> Với đột nhập, điều kiện chính là hệ thống được armed, PIR phát hiện chuyển động và cảm biến siêu âm xác nhận vật ở gần liên tục. Với phá hoại, điều kiện là LDR bị che **và** vật ở rất gần cảm biến siêu âm trong ba giây. Khi có cảnh báo, hệ thống kích hoạt còi, LED đỏ, gửi Telegram và chụp ảnh.

> Sau sabotage, Apps Script tiếp tục theo dõi heartbeat. Nếu heartbeat vẫn đều, trạng thái giữ là sabotage. Nếu heartbeat quá hạn, trạng thái được nâng thành critical compromise và gửi email xác nhận. Nút reset sẽ xóa các cảnh báo cục bộ và đóng event hiện tại.

## Slide 15 - Hình ảnh dự án

> Đây là một số hình ảnh của mô hình thực tế, bao gồm board, camera, cảm biến và bố trí thử nghiệm. Trong quá trình kiểm thử, nhóm không chỉ kiểm tra giao diện dashboard mà còn quan sát log serial, trạng thái Arduino Cloud, Telegram và Apps Script để đối chiếu một sự kiện từ đầu đến cuối.

> Phần tiếp theo, bạn Nguyễn Nhật Anh sẽ trình bày sáu kịch bản demo, kết quả đạt được và hướng phát triển của dự án.

---

# Thành viên 4 - Nguyễn Nhật Anh

**Phụ trách:** Slide 16-20  
**Mục tiêu:** Trình bày demo, kết quả, giới hạn và kết thúc bài thuyết trình.

## Slide 16 - Sáu kịch bản demo

> Nhóm xây dựng sáu kịch bản demo. DS-01 là tự động bật chống trộm theo lịch RTC. DS-02 là phát hiện đột nhập bằng PIR và cảm biến siêu âm, sau đó bật cảnh báo và gửi ảnh qua Telegram.

> DS-04 là phát hiện hành vi che hoặc can thiệp thiết bị bằng điều kiện LDR bị che kết hợp vật áp sát cảm biến. DS-03 là giai đoạn sau DS-04: nếu thiết bị bị mất nguồn hoặc mất liên lạc, Apps Script phát hiện heartbeat quá hạn và gửi email để Parent/Admin xác nhận. Vì DS-03 là hậu quả của DS-04, thứ tự demo thực tế là DS-01, DS-02, DS-04, DS-03, DS-05 và DS-06.

> DS-05 cho phép người dùng bấm nút trên dashboard để chụp ảnh khu vực từ xa. DS-06 là SOS: trẻ em hoặc người lớn có thể kích hoạt khẩn cấp; sau xác nhận, Apps Script gửi escalation đến contact mô phỏng. Trong demo, địa chỉ được dùng là địa chỉ trường Đại học FPT Campus TP.HCM và chỉ xuất hiện trong email escalation sau xác nhận.

[Nếu demo trực tiếp: chạy DS-02 -> DS-04 -> DS-03, sau đó DS-05 và DS-06. Với DS-03, chờ 60-120 giây sau khi ngắt nguồn thiết bị để Apps Script phát hiện timeout.]

## Slide 17 - Kết luận

> Qua dự án, nhóm đã xây dựng được hệ thống an ninh bếp có tính tự động và có khả năng giám sát từ xa. RTC hỗ trợ tự bật/tắt theo lịch; nhiều cảm biến phối hợp để hạn chế báo động giả; camera cung cấp ảnh cho Telegram; dashboard Arduino Cloud hỗ trợ theo dõi và chụp ảnh thủ công.

> Điểm nổi bật là hệ thống không dừng ở việc phát hiện đột nhập. Nó còn phát hiện dấu hiệu phá hoại và theo dõi sự sống của thiết bị bằng heartbeat. Nhờ vậy, nếu thiết bị mất liên lạc sau khi có sabotage, người dùng vẫn nhận được email xác nhận từ Apps Script thay vì mất hoàn toàn dấu vết sự kiện.

## Slide 18 - Hướng phát triển

> Trong tương lai, nhóm có thể cải tiến theo ba hướng. Thứ nhất là thị giác máy tính, ví dụ nhận diện người để giảm false alarm và nhận diện người được phép. Thứ hai là mở rộng phần mềm với ứng dụng di động, cơ sở dữ liệu cloud và nhiều khu vực giám sát độc lập.

> Thứ ba là hạ tầng phần cứng, đặc biệt là pin dự phòng. Hiện tại DS-03 nhận biết việc thiết bị mất liên lạc sau phá hoại, còn pin dự phòng sẽ giúp hệ thống tiếp tục hoạt động ngay cả khi nguồn chính bị ngắt.

## Slide 19 - Lời cảm ơn

> Trên đây là toàn bộ phần trình bày của nhóm về hệ thống IoT chống trộm cho khu vực bếp. Nhóm đã hoàn thiện các luồng giám sát, cảnh báo, chụp ảnh, chống phá hoại, theo dõi heartbeat và SOS ở mức demo học phần.

> Nhóm chúng em xin cảm ơn thầy/cô và các bạn đã lắng nghe.

## Slide 20 - Q&A

> Nhóm chúng em sẵn sàng trả lời câu hỏi về phần cứng, thuật toán phát hiện, Arduino Cloud, Telegram, Google Apps Script hoặc các kịch bản demo.

---

# Gợi ý trả lời nhanh khi được hỏi

| Câu hỏi có thể gặp | Câu trả lời ngắn, thống nhất |
|---|---|
| Vì sao không dùng một cảm biến PIR? | PIR đơn lẻ dễ gây báo động giả. Hệ thống dùng PIR kết hợp siêu âm và điều kiện duy trì theo thời gian để tăng độ tin cậy. |
| DS-04 và DS-03 khác nhau thế nào? | DS-04 phát hiện hành vi đang che/can thiệp thiết bị. DS-03 chỉ xảy ra sau đó nếu Apps Script thấy thiết bị mất heartbeat hoặc suy giảm nghiêm trọng. |
| Heartbeat có tác dụng gì? | ESP32 gửi tín hiệu sống mỗi 30 giây. Apps Script dùng thời điểm server nhận heartbeat cuối để phát hiện thiết bị mất liên lạc sau sabotage. |
| Tại sao email DS-03 không đến ngay? | Timeout là hơn 60 giây và Apps Script quét mỗi một phút, nên thời gian thực tế khoảng 60-120 giây. Điều này tránh kết luận vội từ một lỗi mạng thoáng qua. |
| Bấm xác nhận email có tắt còi không? | Không. Xác nhận chỉ cho phép escalation. Còi, LED và SOS chỉ tắt khi Parent/Admin bấm `reset_alarm`. |
| Demo có gửi cho cơ quan chức năng thật không? | Không. Đây là contact mô phỏng cho mục đích học phần; hệ thống không tự liên hệ cơ quan chức năng thật. |

# Checklist trước khi trình bày

- Kiểm tra RTC đang đúng giờ và WiFi/Arduino Cloud đã kết nối.
- Bật Telegram, Gmail và dashboard trên các thiết bị cần thiết.
- Để mô hình ổn định trước khi thử DS-02, tránh chuyển động ngoài ý muốn.
- Với DS-04, che LDR và đặt vật trong khoảng `<= 15 cm` liên tục ít nhất 3 giây.
- Với DS-03, chỉ ngắt nguồn sau khi DS-04 đã phát cảnh báo; sau đó chờ khoảng 60-120 giây để email đến.
- Sau mỗi cảnh báo, dùng `reset_alarm` trước khi chuyển sang kịch bản độc lập tiếp theo.
