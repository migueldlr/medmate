import { Box, Heading } from '@chakra-ui/core';
import React from 'react';

const LeftRail: React.FC = () => {
  const menuItems = ['overview', 'prescriptions', 'history', 'devices'];
  const selected = 'overview';
  return (
    <Box
      h="100%"
      w="2xs"
      px={8}
      pt={4}
      //   display="flex"
      //   flexDirection="column"
      //   justifyContent="center"
    >
      <Heading mb={8} color="blue.700">
        medmate
      </Heading>
      {menuItems.map((menuItem) => (
        <Heading
          size="lg"
          color={menuItem === selected ? 'black ' : 'gray.400'}
          mb={1}
          key={menuItem}
        >
          {menuItem}
        </Heading>
      ))}
    </Box>
  );
};

export default LeftRail;
