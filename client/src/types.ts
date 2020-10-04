export interface User {
  l_name: string;
  f_name: string;
}

export interface Prescription {
  name: string;
  amt: number;
  u: string;
  times: string[];
  _id: string;
}

export interface Alert {
  time: Date;
  type: string;
  prescription: string;
}

export interface HistoryItem {
  prescription: string;
  time: Date;
}
