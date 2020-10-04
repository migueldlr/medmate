import { Box, Heading, PseudoBox, Text } from '@chakra-ui/core';
import React from 'react';
import { Prescription } from '../types';
import { formatTimeList } from '../util';

interface Props {
  prescription: Prescription;
}

const PrescriptionCard: React.FC<Props> = (props: Props) => {
  const p = props.prescription;
  return (
    <PseudoBox
      as={Box}
      borderRadius={10}
      p={6}
      w="sm"
      bg="white"
      border="1px black"
      transition="linear 0.1s"
      _hover={{ bg: 'gray.100' }}
    >
      <Heading size="lg">{p.name}</Heading>
      <Text>
        {p.amt} {p.u}
        {p.times.length > 0 ? ` at ${formatTimeList(p.times)}` : ''}
      </Text>
    </PseudoBox>
  );
};

export default PrescriptionCard;
