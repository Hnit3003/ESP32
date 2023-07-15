# Báo cáo tuần 1 (10/07 - 16/07)

## Task

### Các state của Task

![Alt text](image-3.png)

- **Ready**: Task sẵn sàng vào trạng thái Running. Các Task này chưa được thực thi vì có một Task khác đang chạy với mức ưu tiên cao hơn hoặc bằng nó.

- **Blocked**: Trạng thái Task chờ, muốn Running thì phải đưa về trạng thái Ready do các event tạm thời hoặc từ bên ngoài. Một Task Running có thể đưa về Blocked bằng các hàm API (Application Program Interface). Các Task Blocked thường có timeout, hết timeout thì Task sẽ unblocked cho dù event chưa xảy ra. Task có thể vào Blocked để đợi Queue, semaphore, event group, semaphore event.

- **Suspended**: Trạng thái treo, hoãn không cho hoạt động, để có thể Running thì phải đưa về trạng thái Ready bằng hàm *vTaskResume()*. Một Task Running, Ready, Blocked có thể đưa về Suspended bằng hàm *vTaskSuspend()*. Task Suspended không có timeout để đưa về Ready như Blocked.

- **Runing**: Task đang chạy (executing in CPU). Task này hiện đang sử dụng bộ xử lý.

Từ State Running &rarr; Not Running: "switched in", "swapped in".

Từ State Not Running &rarr; Running: "switched out", "swapped out".

### Task Prioritise

Mỗi Task được chỉ định mức độ ưu tiên từ `0 đến (configMAX_PRIORITIES - 1)`(FreeRTOSConfig.h).

```C
/* This has impact on speed of search for highest priority */
#define configMAX_PRIORITIES                            ( 25 )
```
**Số ưu tiên càng thấp thì mức ưu tiên càng thấp**. Idle Task có mức ưu tiên là 0 (tskIDLE_PRIORITY).

FreeRTOS luôn đảm bảo rằng các Task Ready hay Running là các Task có mức ưu tiên cao thay vì các Task có mức ưu tiên thấp.

Nhiều Task có thể có cùng chung một mức ưu tiên, và các Task này sẽ tuần hoàn với cùng một khoảng thời gian.

### Task Scheduling

Mặc định, FreeRTOS sử dụng chuẩn `fixed-priority preemptive scheduling`, `round-robin time-sliced` cho các Task có mức độ ưu tiên giống nhau.

- `fixed-priority`: scheduler sẽ không thay đổi mức ưu tiên của các Task.

- `preemptive`: scheduler sẽ luôn chạy các Task có mức ưu tiên cao nhất có thể chạy. Ví dụ khi ngắt (ISR) xảy ra làm thay đổi mức độ ưu tiên cao nhất của Task, scheduler sẽ dừng Task Running có mức ưu tiên thấp hơn và bắt đầu Running Task ISR, bất kể Task có đang trong time slice của nó.

- `round-robin`: các Task có cùng mức ưu tiên sẽ lần lượt vào Running state.

- `time-sliced`: scheduler sẽ switch giữa các Task có mức ưu tiên giống nhau mỗi lần xuất hiện tick interrupt. Khoảng thời gian giứa hai tick interrupt liên tiếp là một time-sliced.

Các Task có mức độ ưu tiên cao nên là các Task hướng sự kiện (event-driven). Để khi các Task này cần đợi event thì sẽ vào Blocked state, và các Task có mức ưu tiên thấp hơn sẽ được Running. Tránh tình trạng chỉ chạy các Task có mức ưu tiên cao và bỏ lỡ đi các Task mức ưu tiên thấp.

**Config FreeRTOS scheduling policy**

Trong file FreeRTOSConfig.h:

- `configUSE_PREEMPTION`:
  
  - `0`: quyền ưu tiên bị tắt (Preemption), switched Task chỉ xảy ra khi Task đang ở Running state switched out sang Block hoặc Suspended. Để switch Task thì Task đang Running phải gọi `taskYIELD()`, hoặc yêu cầu chuyển đổi ngữ cảnh ISR theo cách thủ công.

- `configUSE_TIME_SLICING`:

  - `0`: time-sliced bị tắt, các Task có mức ưu tiên giống nhau sẽ không switch với nhau mỗi lần tick interrupt.