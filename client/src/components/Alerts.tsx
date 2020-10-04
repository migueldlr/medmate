import { Box } from '@chakra-ui/core';
import { compareAsc } from 'date-fns/esm';
import React, { useEffect, useState } from 'react';
import { user } from '../firebase/firebase';
import { Alert, Prescription } from '../types';
import AlertBox from './AlertBox';

interface AlertData {
  alert: Alert;
  pname?: string;
  remove: () => void;
}

const Alerts: React.FC = () => {
  const [alerts, setAlerts] = useState<AlertData[]>([]);

  useEffect(() => {
    (async (): Promise<void> => {
      const p = await user.collection('prescriptions').get();
      const prescriptions = p.docs.map(
        (d) => ({ ...d.data(), _id: d.id } as Prescription)
      );
      console.log(prescriptions);

      user.collection('alerts').onSnapshot((doc) => {
        setAlerts(
          doc.docs.map((d) => {
            console.log(d.data().prescription);
            return {
              alert: { ...d.data(), time: d.data().time.toDate() },
              pname: prescriptions.find((p) => p._id === d.data().prescription)
                ?.name,
              remove: () => {
                user.collection('alerts').doc(d.id).delete();
              },
            } as AlertData;
          })
        );
      });
    })();
  }, []);

  alerts.sort((a, b) => compareAsc(a.alert.time, b.alert.time));

  return (
    <Box>
      {alerts.map(({ alert, pname, remove }, i) => {
        return (
          <Box key={i} mb={2}>
            <AlertBox alert={alert} pname={pname} remove={remove} />
          </Box>
        );
      })}
    </Box>
  );
};

export default Alerts;
