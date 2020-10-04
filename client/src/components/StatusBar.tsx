import { Box, Heading } from '@chakra-ui/core';
import { FaUserCircle } from 'react-icons/fa';
import React from 'react';
import { User } from '../types';

interface Props {
  user: User;
}

const StatusBar: React.FC<Props> = (props: Props) => {
  return (
    <Box display="flex" flexDirection="row">
      <Heading size="md" color="gray.600" mr={2}>
        {props.user.f_name}
      </Heading>
      <Box size={6} as={FaUserCircle} color="gray.600" />
    </Box>
  );
};

export default StatusBar;
