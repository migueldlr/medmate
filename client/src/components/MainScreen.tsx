import { Box, Divider, Heading, Text } from '@chakra-ui/core';
import React, { useEffect, useState } from 'react';
import { user } from '../firebase/firebase';
import LeftRail from './LeftRail';
import StatusBar from './StatusBar';
import { HistoryItem, Prescription, User } from '../types';
import Alerts from './Alerts';
import PrescriptionBox from './PrescriptionBox';
import HistoryBox from './HistoryBox';

const MainScreen: React.FC = () => {
  const [prescriptions, setPrescriptions] = useState<Prescription[]>([]);
  const [hist, setHist] = useState<HistoryItem[]>([]);
  const [userInfo, setUserInfo] = useState<User>();

  useEffect(() => {
    user.onSnapshot((doc) => {
      const data = doc.data();
      if (data != null) {
        setUserInfo(data as User);
      }
    });

    user.collection('prescriptions').onSnapshot((doc) => {
      setPrescriptions(
        doc.docs.map((d) => ({ ...d.data(), _id: d.id } as Prescription))
      );
    });

    user.collection('history').onSnapshot((doc) => {
      setHist(
        doc.docs.map(
          (d) => ({ ...d.data(), time: d.data().time.toDate() } as HistoryItem)
        )
      );
    });
  }, []);

  return (
    <Box
      h="100vh"
      w="100vw"
      display="flex"
      flexDirection="row"
      position="relative"
    >
      <LeftRail />
      <Divider
        orientation="vertical"
        borderLeft="0.1rem solid"
        m={0}
        boxSizing="border-box"
      />
      <Box bg="gray.200" p={8} pt={2} w="full">
        <Heading mb={10} size="2xl">
          overview
        </Heading>
        <Alerts />
        <Heading size="lg" mb={4}>
          you have {prescriptions.length} prescription
          {prescriptions.length === 1 ? '' : 's'}:
        </Heading>
        <Box mb={4}>
          <PrescriptionBox prescriptions={prescriptions} />
        </Box>
        {/* <Text>{JSON.stringify(prescriptions)}</Text> */}
        <Heading size="lg" mb={4}>
          history:
        </Heading>
        <Box>
          <HistoryBox hist={hist} prescriptions={prescriptions} />
        </Box>
      </Box>
      <Box position="absolute" top="1.75rem" right={8}>
        {userInfo != null && <StatusBar user={userInfo} />}
      </Box>
    </Box>
  );
};

export default MainScreen;
