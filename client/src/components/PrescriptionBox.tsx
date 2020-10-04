import { Box, PseudoBox } from '@chakra-ui/core';
import React, { useState } from 'react';
import { Prescription } from '../types';
import PrescriptionCard from './PrescriptionCard';
import { MdAddCircleOutline } from 'react-icons/md';
import CreatePrescriptionDialog from './CreatePrescriptionDialog';

interface Props {
  prescriptions: Prescription[];
}

const PrescriptionBox: React.FC<Props> = ({ prescriptions }: Props) => {
  const [showCreateDialog, setShowCreateDialog] = useState(false);

  return (
    <Box mb={4} display="flex" alignItems="center" flexWrap="wrap">
      {prescriptions.map((p) => (
        <Box key={p._id} mr={2} mb={2}>
          <PrescriptionCard prescription={p} />
        </Box>
      ))}
      <PseudoBox
        size={6}
        as={MdAddCircleOutline}
        color="gray.600"
        cursor="pointer"
        onClick={(): void => {
          setShowCreateDialog(true);
        }}
        transition="0.1s linear"
        _hover={{ color: 'gray.800' }}
      />
      <CreatePrescriptionDialog
        isOpen={showCreateDialog}
        onClose={(): void => {
          setShowCreateDialog(false);
        }}
      />
    </Box>
  );
};

export default PrescriptionBox;
