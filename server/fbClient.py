import firebase_admin
from firebase_admin import credentials, firestore
from mmglobals import active_users

def init_fb():
    cred = credentials.Certificate(
        "med-mate-4d659-firebase-adminsdk-vj3mx-1fb1a609f4.json")
    firebase_admin.initialize_app(cred)

def add_user_devs(user_uid):
    db = firestore.client()
    user_dict = db.document(f'users/{user_uid}').get().to_dict()
    active_users[user_uid]['devs'] = user_dict['devs']
    print(active_users[user_uid]['devs'])

def get_dev_info(dev_name):
    db = firestore.client()
    dev_dict = db.document(f'devices/{dev_name}').get().to_dict()
    return dev_dict

def get_pid_for_dev(dev_name, user_id):
    db = firestore.client()
    pres = db.collection(f'users/{user_id}/prescriptions').where(u'device', u'==', dev_name).stream()
    for p in pres:
        return p.id

def get_pres_info(user_id, pres_id):
    db = firestore.client()
    return db.document(f'users/{user_id}/prescriptions/{pres_id}').get().to_dict()

def post_history_item(uid, pid, time):
    db = firestore.client()
    print(f'uid: {uid}')
    user_ref = db.document(f'users/{uid}')
    user_dict = user_ref.get().to_dict()
    user_dict['evtct'] += 1
    user_ref.update({'evtct': user_dict['evtct']})
    evt_builder = {
        'prescription': pid,
        'time': time
    }
    print(f'posting new history item:\n{evt_builder}')
    db.collection(f'users/{uid}/history').add(evt_builder)

def post_alert(uid, pid, time, status):
    db = firestore.client()
    alert = {'prescription': pid,
        "type": status,
        "time": time
        }
    print(f'posting new alert:\n{alert}')
    db.collection(f'users/{uid}/alerts').add(alert)

def update_a_date(dev_name, uid, pres_id, pres_dict):
    db = firestore.client()
    pres_ref = db.document(f'users/{uid}/prescriptions/{pres_id}')
    print(f'users/{uid}/prescriptions/{pres_id}', pres_ref)
    print(f"updating active date/time to {pres_dict['a_date']}/{pres_dict['a_time']}")
    pres_ref.update({'a_date': pres_dict['a_date'], 'a_time': pres_dict['a_time']})