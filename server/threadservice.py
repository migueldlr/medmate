import queue
import msgProcess
from mmglobals import active_users, connected_devices
from datetime import datetime, timedelta, date, time
import datetime as dt
import threading
from time import sleep

num_threads = 100
thread_info = {}
thread_idx_list = queue.Queue()

def init_threads():
    for i in range(num_threads):
        thread_info[i] = {'active': False}
        thread_idx_list.put(i)

def waiting_to_take(dev_name, thread_id):
    while True:
        now = dt.datetime.now()
        print(f"waiting, tid {thread_id} now {now.isoformat()}")
        if connected_devices[dev_name]['thread_id'] != thread_id:
            thread_idx_list.put(thread_id)
            print('thread overruled, closing thread')
            return
        if thread_info[thread_id]['ttt'] < now:
            print(f'Next time to take for {dev_name} is now')
            connected_devices[dev_name]['readyToTake'] = True
            msgProcess.send_ready_to_take(dev_name)
            connected_devices[dev_name]['thread_id'] = None
            thread_idx_list.put(thread_id)
            return
        sleep(5)

def start_wait(dev_name, next_ttt):
    if thread_idx_list.empty():
        print("FATAL ERROR: NO AVAILABLE THREADS")
        return
    tid = thread_idx_list.get()
    while thread_info[tid]['active']:
        print("FATAL ERROR: GOT ACTIVE THREAD")
        tid = thread_idx_list.get()

    thread_info[tid]['active'] = True
    thread_info[tid]['ttt'] = next_ttt
    connected_devices[dev_name]['thread_id'] = tid
    x = threading.Thread(target=waiting_to_take, args=(dev_name, tid,))
    x.setDaemon(True)
    x.start()

def date_from_ts(date_str):
    y = date_str[0:4]
    m = date_str[4:6]
    d = date_str[6:]
    return dt.date.fromisoformat(f'{y}-{m}-{d}')

def times_from_ts(time_strs):
    times = []
    for time_str in time_strs:
        times.append(dt.time(int(time_str[0:2]), int(time_str[2:]), 0))
    return times

def set_time_to_take(dev_name, pres_dict):
    debugBypass = False

    now = dt.datetime.now()
    a_date = date_from_ts(pres_dict['a_date'])
    print(a_date)
    a_time = times_from_ts([pres_dict['a_time']])[0]
    print(a_time)
    times = times_from_ts(pres_dict['times'])
    step = pres_dict['step']

    daysAway = 0
    next_ttt = None
    while True: 
        for time in times:
            a_ts = dt.datetime.combine(a_date, a_time)
            curr = dt.datetime.combine(a_date, time) + timedelta(days=daysAway)
            if curr > a_ts and curr > now:
                next_ttt = curr
                break
        if next_ttt is not None:
            break
        daysAway += step
    if debugBypass:
        next_ttt = dt.datetime.now() + timedelta(seconds=6)
    print(f'next time to take for {dev_name}: {next_ttt.isoformat()}')
    start_wait(dev_name, next_ttt)
    return next_ttt
    
    