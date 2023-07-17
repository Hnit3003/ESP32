# Báo cáo tuần 1 (10/07/23 - 16/07/23)

Đã làm được trong tuần 1:

- Tìm hiểu về FreeRTOS: State của Task, Scheduler, khởi tạo Task, Heap Memory Management (em chưa ghi vào đây).
- Code ESP32: GPIO cơ bản, ngắt ngoài, tạo task, Running 2 task.
- MarkDown cơ bản.
- Học Git anh Nhật.

Kế hoạch tuần 2:

- Tìm hiểu FreeRTOS: tiếp tục tìm hiểu phần tiếp theo.
- Code ESP32: Code được thư viện nút nhấn. Code thư viện các ngoại vi.
- Xem design partern.

# Task

## Các state của Task (Statess of Task)

![Alt text](image-3.png)

- **Ready**: Task sẵn sàng vào trạng thái Running. Các Task này chưa được thực thi vì có một Task khác đang chạy với mức ưu tiên cao hơn hoặc bằng nó.

- **Blocked**: Trạng thái Task chờ, muốn Running thì phải đưa về trạng thái Ready do các event tạm thời hoặc từ bên ngoài. Một Task Running có thể đưa về Blocked bằng các hàm API (Application Program Interface). Các Task Blocked thường có timeout, hết timeout thì Task sẽ unblocked cho dù event chưa xảy ra. Task có thể vào Blocked để đợi Queue, semaphore, event group, semaphore event.

- **Suspended**: Trạng thái treo, hoãn không cho hoạt động, để có thể Running thì phải đưa về trạng thái Ready bằng hàm *vTaskResume()*. Một Task Running, Ready, Blocked có thể đưa về Suspended bằng hàm *vTaskSuspend()*. Task Suspended không có timeout để đưa về Ready như Blocked.

- **Runing**: Task đang chạy (executing in CPU). Task này hiện đang sử dụng bộ xử lý.

Từ State Running &rarr; Not Running: "switched in", "swapped in".

Từ State Not Running &rarr; Running: "switched out", "swapped out".

## Task Prioritise

Mỗi Task được chỉ định mức độ ưu tiên từ `0 đến (configMAX_PRIORITIES - 1)`(FreeRTOSConfig.h).

```C
/* This has impact on speed of search for highest priority */
#define configMAX_PRIORITIES                            ( 25 )
```
**Số ưu tiên càng thấp thì mức ưu tiên càng thấp**. Idle Task có mức ưu tiên là 0 (tskIDLE_PRIORITY).

FreeRTOS luôn đảm bảo rằng các Task Ready hay Running là các Task có mức ưu tiên cao thay vì các Task có mức ưu tiên thấp.

Nhiều Task có thể có cùng chung một mức ưu tiên, và các Task này sẽ tuần hoàn với cùng một khoảng thời gian.

## Task Scheduling

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

## Tạo Task (Implementing a Task)

### Cấu trúc của một Task

```C
void vTaskFunction(void *pvParameters)
{
  while(1)
  {
    //Task application
  }

  //If Task need to exit 
  vTaskDelete(pxCreatedTask);
}
```

Khi khởi tạo Task, hàm vTaskFunction() sẽ có kiểu TaskFunction_t, được define là hàm void, và thông số đầu vào là con trỏ void.

Thông thường các Task nên được viết theo kiểu event-driven để cho bộ xử lý có thể chạy Task Idle, và để các Task có mức độ ưu tiên thấp hơn được chạy.

Cấu trúc thông thường của một Task event-driven:

```C
void vATaskFunction(void *pvParameters)
{
  while(1)
  {
    /* Psudeo code showing a task waiting for an event  with a block time. 
    If the event occurs, process it. If the timeout expires before the event occurs, then the system may be in an error state, so handle the error.  
    Here the pseudo code "WaitForEvent()" could replaced with xQueueReceive(), ulTaskNotifyTake(), xEventGroupWaitBits(), or any of the other FreeRTOS communication and synchronisation primitives. */
    if( WaitForEvent( EventObject, TimeOut ) == pdPASS )
    {
        // Handle event here.
    }
    else
    {
        // Clear errors, or take actions here.
    }
  }
  vTaskDelete( NULL );
}
```
### **Hàm tạo Task**

#### **`TaskHandle_t`**

> Type của Task được tham chiếu. Ví dụ: hàm xTaskCreate trả về (dạng pointer) là biến TaskHandle_t, biến này có thể sử dụng làm tham số cho hàm vTaskDelete() để xóa tác vụ.

#### **`xTaskCreate()`**
 
 > Dùng để tạo một Task mới và thêm Task này vào list của Ready state.

```C
BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
                        const char * const pcName,
                        configSTACK_DEPTH_TYPE usStackDepth,
                        void *pvParameters,
                        UBaseType_t uxPriority,
                        TaskHandle_t *pxCreatedTask );
```


*Parameter:*
- **`pvTaskCode`**: Trỏ đến hàm của Task (tên của hàm khởi tạo Task).

- **`pcName`**: Discription name của Task, chủ yếu dùng để debug. Độ dài tối đa của `pcName` được define `configMAX_TASK_NAME_LEN` trong FreeRTOSConfig.h

  ```C
  #ifndef configMAX_TASK_NAME_LEN
    #define configMAX_TASK_NAME_LEN    16
  ```

- **`usStackDepth`**: Dung lượng để phân bổ cho Stack của Task. Theo FreeRTOS thì tham số này là word, còn FreeRTOS trong esp32 tính theo byte.

- **`pvParameters`**: Tham số truyền vào hàm của Task.

- **`uxPriority`**: Mức độ ưu tiên của Task.

- **`pxCreatedTask`**: Được sử dụng để handle các tác vụ liên quan đến Task. Thường sẽ khởi tạo tham số này bằng NULL.

  ```C
    TaskHandle_t xHandle = NULL;
    xTaskCreate(..., &xHandle);

    //Use the Task's Handle to delete a Task
    vTaskDelete(xHandle);
  ```

*Return:*

- Nếu tạo Task thành công thì `pdPASS` sẽ được trả về.

- Nếu tạo Task khôn thành công thì sẽ trả về `errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY`.

#### **`vTaskDelele()`**

> Xóa một Task khỏi Kernel RTOS management. Task sẽ bị xóa khỏi tất cả các list của Ready, Blocked, Suspended, Event.

> Lưu ý: Idle Task sẽ giải phóng bộ nhớ của các Task bị xóa được cấp phát trong Kernel RTOS. Vì vậy Idle Task phải đảm bảo được Running. Bộ nhớ được cấp phát cho Task không được giải phóng tự động và phải được free trước khi Task được xóa.

```C
void vTaskDelete( TaskHandle_t xTask );
```
*Parameters:*

- **`xTask`**: Handle của Task bị xóa. Nếu truyền vào NULL làm cho Task đang gọi bị xóa.

